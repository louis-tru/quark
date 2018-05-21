// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

'use strict';

const NativeModule = require('native_module');
const util = require('util');
const internalModule = require('internal/module');
const vm = require('vm');
const assert = require('assert').ok;
const fs = require('fs');
const path = require('path');

// We use this alias for the preprocessor that filters it out
const debug = util.debuglog('module');

// Resolved path to process.argv[1] will be lazily placed here
// (needed for setting breakpoint when called with --inspect-brk)
var resolvedArgv;
var modulePaths = [];
var packages = null;
var isMain = false;

const wrapper = [
  '(function (exports, require, module, __filename, __dirname, resolve) { ', '\n});'
];

/**
 * @class Module
 */
class Module {

  constructor(id, parent, pkg) {
    this.id = id;
    this.exports = { };
    this.parent = parent;
    this.filename = null;
    this.loaded = false;
    this.children = [];
    this.package = pkg;
    this.file = '';
    this.dir = '';
  }

  // Given a file name, pass it to the proper extension handler.
  load(filename) {
    debug('load %j for module %j', filename, this.id);

    if ( isMain ) {
      isMain = false;
      assert(!process.mainModule);
      process.mainModule = this;
      this.id = '.';
    }

    assert(!this.loaded);
    this.filename = filename;
    this.paths = [];

    var extension = path.extname(filename).replace(/\?.*$/, '') || '.js';
    if (!Module._extensions[extension]) extension = '.js';
    Module._extensions[extension](this, filename);
    this.loaded = true;
  }

  // Loads a module at the given file path. Returns that module's
  // `exports` property.
  require(path) {
    assert(path, 'missing path');
    assert(typeof path === 'string', 'path must be a string');
    return Module._load(path, this, /* isMain */ false);
  }

  set _export(value) {
    Object.getOwnPropertyNames(value).forEach((name)=>{
      Object.defineProperty(this.exports, name, Object.getOwnPropertyDescriptor(value, name));
    });
  }

  // Run the file contents in the correct scope or sandbox. Expose
  // the correct helper variables (require, module, exports) to
  // the file.
  // Returns exception, if any.
  _compile(content, filename, filename2) {

    content = internalModule.stripShebang(content);

    // create wrapper function
    var wrapper = Module.wrap(content);

    var compiledWrapper = vm.runInThisContext(wrapper, {
      filename: filename,
      lineOffset: 0,
      displayErrors: true
    });

    var inspectorWrapper = null;
    if (process._breakFirstLine && process._eval == null) {
      if (!resolvedArgv) {
        // we enter the repl if we're not given a filename argument.
        if (process.argv[1]) {
          resolvedArgv = Module._resolveFilename(process.argv[1], null, false);
        } else {
          resolvedArgv = 'repl';
        }
      }

      // Set breakpoint on module start
      if (filename2 === resolvedArgv) {
        delete process._breakFirstLine;
        inspectorWrapper = process.binding('inspector').callAndPauseOnStart;
        if (!inspectorWrapper) {
          const Debug = vm.runInDebugContext('Debug');
          Debug.setBreakPoint(compiledWrapper, 0, 0);
        }
      }
    }
    var dirname = path.dirname(filename);
    var require = internalModule.makeRequireFunction(this);
    var result;
    if (inspectorWrapper) {
      result = inspectorWrapper(compiledWrapper, this.exports, this.exports,
                                require, this, filename, dirname, require.resolve);
    } else {
      result = compiledWrapper.call(this.exports, this.exports, require, this,
                                    filename, dirname, require.resolve);
    }
    return result;
  }

  /* 禁用旧的模块载入模式 */
  static _findPath(request, paths, isMain) { return false }
  static _nodeModulePaths(from) { return [] }
  static _resolveLookupPaths(request, parent, newReturn) { return [] }

  static _load(request, parent, isMain) {
    return packages._require(request, parent);
  }
  static _resolveFilename(request, parent, isMain) {
    return packages._resolveFilename(request, parent);
  }

  // bootstrap main module.
  static runMain() {
    isMain = true;
    // Load the main module--the command line argument.
    packages._start();
    // Handle any nextTicks added in the first tick of the program
    process._tickCallback();
  }

  static _initPaths() {
    const isWindows = process.platform === 'win32';

    var homeDir;
    if (isWindows) {
      homeDir = process.env.USERPROFILE;
    } else {
      homeDir = process.env.HOME;
    }

    // $PREFIX/lib/node, where $PREFIX is the root of the Node.js installation.
    var prefixDir;
    // process.execPath is $PREFIX/bin/node except on Windows where it is
    // $PREFIX\node.exe.
    if (isWindows) {
      prefixDir = path.resolve(process.execPath, '..');
    } else {
      prefixDir = path.resolve(process.execPath, '..', '..');
    }
    var paths = [path.resolve(prefixDir, 'lib', 'node')];

    if (homeDir) {
      paths.unshift(path.resolve(homeDir, '.node_libraries'));
      paths.unshift(path.resolve(homeDir, '.node_modules'));
    }

    var nodePath = process.env['NODE_PATH'];
    if (nodePath) {
      paths = nodePath.split(path.delimiter).filter(function(path) {
        return !!path;
      }).concat(paths);
    } else {
      nodePath = [];
    }

    modulePaths = nodePath;
  }

  static get globalPaths() {
    return modulePaths;
  }

  static get wrapper() {
    return wrapper;
  }

  static wrap(script) {
    return wrapper[0] + script + wrapper[1];
  }

  static _preloadModules(requests) {
    if (!Array.isArray(requests))
      return;

    // Preloaded modules have a dummy parent module which is deemed to exist
    // in the current working directory. This seeds the search path for
    // preloaded modules.
    var parent = new Module('internal/preload', null);
    try {
      parent.paths = Module._nodeModulePaths(process.cwd());
    } catch (e) {
      if (e.code !== 'ENOENT') {
        throw e;
      }
    }
    for (var n = 0; n < requests.length; n++)
      parent.require(requests[n]);
  }
}

Module._cache = { };
Module._pathCache = { };
Module._extensions = { };
Module._debug = debug;

// backwards compatibility
Module.Module = Module;

module.exports = Module;

Module._initPaths();

packages = require('pkg');
