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

const win32 = process.platform == 'win32';
const { readFile, readFileSync, 
				existsSync, isFileSync, 
				isDirectorySync, readdirSync } = process.binding('shark_reader');
const internalURL = require('internal/url');
const getPathFromURLRaw = internalURL.getPathFromURL;
const _util = process.binding('shark_util');

const fallbackPath = win32 ? function(url) {
  return url.replace(/^file:\/\/(\/([a-z]:))?/i, '$3').replace(/\//g, '\\');
} : function(url) {
  return url.replace(/^file:\/\//i, '');
};

const join_path = win32 ? function(args) {
  for (var i = 0, ls = []; i < args.length; i++) {
    var item = args[i];
    if (item) ls.push(item.replace(/\\/g, '/'));
  }
  return ls.join('/');
}: function(args) {
  for (var i = 0, ls = []; i < args.length; i++) {
    var item = args[i];
    if (item) ls.push(item);
  }
  return ls.join('/');
};

const matchs = win32 ? {
  resolve: /^((\/|[a-z]:)|([a-z]{2,}:\/\/[^\/]+)|((file|zip):\/\/\/))/i,
  is_absolute: /^([\/\\]|[a-z]:|[a-z]{2,}:\/\/[^\/]+|(file|zip):\/\/\/)/i,
  is_local: /^([\/\\]|[a-z]:|(file|zip):\/\/\/)/i,
} : {
  resolve: /^((\/)|([a-z]{2,}:\/\/[^\/]+)|((file|zip):\/\/\/))/i,
  is_absolute: /^(\/|[a-z]{2,}:\/\/[^\/]+|(file|zip):\/\/\/)/i,
  is_local: /^(\/|(file|zip):\/\/\/)/i,
};

/** 
 * format part 
 */
function resolve_path_level(path, retain_up) {
  var ls = path.split('/');
  var rev = [];
  var up = 0;
  for (var i = ls.length - 1; i > -1; i--) {
    var v = ls[i];
    if (v && v != '.') {
      if (v == '..') // set up
        up++;
      else if (up === 0) // no up item
        rev.push(v);
      else // un up
        up--;
    }
  }
  path = rev.reverse().join('/');

  return (retain_up ? new Array(up + 1).join('../') + path : path);
}

const PREFIX = 'file:///';

/**
 * return format path
 */
function resolve() {
  var path = join_path(arguments);
  var prefix = '';
  // Find absolute path
  var mat = path.match(matchs.resolve);
  var slash = '';
  
  // resolve: /^((\/|[a-z]:)|([a-z]{2,}:\/\/[^\/]+)|((file|zip):\/\/\/))/i,
  // resolve: /^((\/)|([a-z]{2,}:\/\/[^\/]+)|((file|zip):\/\/\/))/i,
  
  if (mat) {
    if (mat[2]) { // local absolute path /
      if (win32 && mat[2] != '/') { // windows d:\
        prefix = PREFIX + mat[2] + '/';
        path = path.substr(2);
      } else {
        prefix = PREFIX; //'file:///';
      }
    } else {
      if (mat[4]) { // local file protocol
        prefix = mat[4];
      } else { // network protocol
        prefix = mat[0];
        slash = '/';
      }
      if (prefix == path.length) // file:///
        return prefix;
      path = path.substr(prefix.length);
    }
  } else { // Relative path, no network protocol
    var cwd = process.cwd();
    prefix = PREFIX; // 'file:///';
    path = (win32 ? cwd.substr(3) : cwd) + '/' + path;
  }

  path = resolve_path_level(path);

  return path ? prefix + slash + path : prefix;
}

/**
 * @func is_absolute # 是否为绝对路径
 */
function is_absolute(path) {
  return matchs.is_absolute.test(path);
}

/**
 * @func is_local # 是否为本地路径
 */
function is_local(path) {
  return matchs.is_local.test(path);
}

function is_local_zip(path) {
  return /^zip:\/\/\//i.test(path);
}

function is_network(path) {
  return /^(https?):\/\/[^\/]+/i.test(path);
}

function resolveMainPath(path) {
  if (path) {
    if ( !is_absolute(path) ) {
      // 非绝对路径,优先查找资源路径
      if (isFileSync(_util.resources(path + '/package.json'))) {  
        // 如果在资源中找到`package.json`文件
        path = _util.resources(path);
      }
    }
    path = fallbackPath(resolve(path));
  }
  return path;
}

function getPathFromURL(path) {
  if ( typeof path == 'string' ) {
    return fallbackPath(path);
  } else {
    return getPathFromURLRaw(path)
  }
}

function maybeCallback(cb) {
  return typeof cb === 'function' ? cb : function(err) {
  	if (err) throw err;
  };
}

function readFile2(path, options, callback) {
	var callback = maybeCallback(callback || options);

	var cb = (function(result) {
		callback(null, result);
	}).catch(function(err) {
		callback(err);
	});

	if ( typeof options == 'function' || !options ) {
		readFile(path, cb);
	} else {
		readFile(path, options, cb);
	}
}

module.exports = {
	readFile,
	readFileSync,
	existsSync,
	isFileSync,
	isDirectorySync,
	readdirSync,
	readFile2,
	fallbackPath,
	getPathFromURL,
	resolve_path_level,
	resolve,
	is_absolute,
	is_local,
	is_local_zip,
	is_network,
  resolveMainPath,
};