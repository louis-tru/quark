/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

const _util = __require__('_util');
const _path = __require__('_path');
const _fs = __require__('_fs');
const _http = __require__('_http');
const _pkgutil = __require__('_pkgutil').default;
const { fallbackPath,
				resolve, isAbsolute, isLocal, isLocalZip, stripShebang,
				isNetwork, assert, stripBOM, makeRequireFunction } = _pkgutil;
const { readFile, readFileSync, isFileSync, 
				isDirectorySync, readdirSync, existsSync } = __require__('_reader');
const { haveNode } = _util;
const isWindows = _util.platform == 'win32';
const debug = _pkgutil.debugLog('PKG');
const _debug = _util.debug as boolean;

type Optopns = Dict<string|string[]>;

function format_msg(args: any[]) {
	var msg = [];
	for (var i = 0; i < args.length; i++) {
		var arg = args[i];
		msg.push( arg instanceof Error ? arg.message: arg );
	}
	return msg;
}

function print_warn(...args: any[]) {
	console.warn(...format_msg(args));
}

function new_err(e: any, code?: string) {
	if (! (e instanceof Error)) {
		if (typeof e == 'object') {
			e = Object.assign(new Error(e.message || 'Unknown error'), e);
		} else {
			e = new Error(e);
		}
	}
	var err = e as Error;
	if (code)
		err.code = code;
	return err;
}

function set_url_args(path: string, arg?: string) {
	if (/^(https?):\/\//i.test(path)) {
		var url_arg = arg || options.url_arg;
		if ( url_arg ) {
			return path + (path.indexOf('?') == -1 ? '?' : '&') + url_arg;
		}
	}
	return path;
}

function readText(path: string) {
	return new Promise<string>(function(resolve, reject) {
		if ('ext://' == path.substring(0, 6)) {
			try {
				var r = _util.__extendModuleContent(path.substring(6));
				assert(r, `Cannot find module ${path}`);
				return resolve(r);
			} catch(err) {
				reject(err);
			}
		} else {
			readFile((err?: Error, r?: any)=>err?reject(err):resolve(r), path, 'utf8');
		}
	});
}

function readTextSync(path: string): string {
	if ('ext://' == path.substring(0, 6)) {
		var r = _util.__extendModuleContent(path.substring(6));
		assert(r, `Cannot find module ${path}`);
		return r;
	} else {
		return readFileSync(path, 'utf8');
	}
}

function throwErr(err: any, cb?: Cb) {
	err = new_err(err);
	if (cb)
		cb(err);
	else
		throw err;
}

function _parseJSON(source: string, filename: string) {
	try {
		return JSON.parse(stripBOM(source));
	} catch (err: any) {
		err.message = filename + ': ' + err.message;
		throw err;
	}
}

function readJSONSync(filename: string) {
	return _parseJSON(readTextSync(filename), filename);
}

async function readJSON(filename: string) {
	return _parseJSON(await readText(filename), filename);
}

function throw_MODULE_NOT_FOUND(request: string, parent?: Module) {
	var msg = `Cannot find module '${request}'`;
	if (parent) {
		msg += ` in module ${parent.filename}`;
	}
	var err = new Error(msg);
	err.code = 'MODULE_NOT_FOUND';
	throw err;
}

function readLocalPackageLinkSync(pathname: string) {
	if (_path.extname(pathname) == '.link') {
		if (isFileSync(pathname)) {
			var link = readTextSync(pathname);
			link = isAbsolute(link) ? resolve(link): resolve(pathname, '..', link);
			if (isDirectorySync(link)) {
				if ( isFileSync(link + '/package.json') ) {
					return link;
				}
			}
		}
	}
}

function std_package_main(main?: string) {
	if (main) {
		main = String(main);
		var i = 0, len = main.length;
		while (i < len && './\\'.indexOf(main[i]) >= 0) {
			i++;
		}
		return i < len ? main.slice(i) : '';
	}
	return '';
}

interface Cb {
	(err?: Error): void;
}

export interface PackageJson extends Dict {
	name: string;
	main: string;
	version: string;
	description?: string;
	scripts?: Dict<string>;
	author?: Dict<string>;
	keywords?: string[];
	license?: string;
	bugs?: Dict<string>;
	homepage?: string;
	devDependencies?: Dict<string>;
	dependencies?: Dict<string>;
	bin?: string;
	hash?: string;
	id?: string;
  app?: string;
}

class ModulePath {
	readonly path: string;
	readonly network: boolean;
	private _packagesJson: Dict<PackageJson> | null = null;
	private _packagesPath = new Map<string, string>();

	constructor(path: string) {
		assert(path);
		assert(typeof path == 'string', 'Search Path type error');
		this.path = resolve(path);
		this.network = isNetwork(path);
	}

	private setPackagesPath(pkgName: string) {
		var json = (this._packagesJson as Dict<PackageJson>)[pkgName] ;
		var path: string;
		var link = json.symlink as string;
		if (link) {
			// delete json.symlink;
			path = isAbsolute(link) ? resolve(link): resolve(this.path, link);
		} else {
			path = this.path + '/' + pkgName;
		}
		this._packagesPath.set(pkgName, path);
	}

	private loadLocalSync() {
		// local read mode
		assert(isDirectorySync(this.path), `"${this.path}" is not a directory`);
		for (var dirent of readdirSync(this.path) ) {
			if (dirent.type == 2/*dir*/ || (dirent.type == 3/*link*/ && isDirectorySync(dirent.pathname))) { // directory
				if ( isFileSync(dirent.pathname + '/package.json') ) {
					this._packagesPath.set(dirent.name, dirent.pathname);
				}
			} else { // has package link
				var link = readLocalPackageLinkSync(dirent.pathname);
				if (link) {
					this._packagesPath.set(dirent.name.substring(0, dirent.name.length - 5), link);
				}
			}
		}
	}

	async load() {
		if (this.network) {
			// load packages.json `packages.json`
			let {resolve} = resolveFilename(this.path + '/packages.json', undefined, true);
			this._packagesJson = await readJSON(resolve) as Dict<PackageJson>;
			Object.keys(this._packagesJson).forEach(k=>this.setPackagesPath(k));
		} else { // local
			this.loadLocalSync();
		}
	}

	loadSync() {
		if (this.network) {
			// load packages.json `packages.json`
			var {resolve: filename} = resolveFilename(this.path + '/packages.json', undefined, true);
			this._packagesJson = readJSONSync(filename) as Dict<PackageJson>; // symlink
			Object.keys(this._packagesJson).forEach(k=>this.setPackagesPath(k));
		} else { // local
			this.loadLocalSync();
		}
	}

	loadSyncWithoutErr() {
		try {
			this.loadSync();
			return true;
		} catch(err) {
			return false;
		}
	}

	hasPackage(name: string) {
		return this._packagesPath.has(name);
	}

	isNetwork(name: string) {
		var path = this._packagesPath.get(name);
		assert(path);
		return isNetwork(path) ? true : false;
	}

	packageJson(name: string): PackageJson {
		var path = this._packagesPath.get(name);
		assert(path, `package ${name} does not exist in ${this.path} path`);
		if (this._packagesJson) {
			return this._packagesJson[name];
		} else { // local
			path = `${path}/package.json`;
			return readJSONSync(path);
		}
	}

	createPackage(pkgName: string) {
		var path = this._packagesPath.get(pkgName) as string;
		var pkg = packages.get(path);
		if (!pkg)
			pkg = new PackageIMPL(path, this.packageJson(pkgName));
		var rawpath = this.path + '/' + pkgName;
		if (!packages.has(rawpath)) {
			packages.set(rawpath, pkg);
		}
		return pkg;
	}

}

const modulePaths: string[] = [];
const modulePathCache: Map<string, ModulePath> = new Map();
const packages: Map<string, PackageIMPL> = new Map();
const lookupCaches: Map<string, LookupResult> = new Map(); // absolute path cache
const options: Optopns = _pkgutil.options;
var mainModule: Module | null = null;

interface LookupResult {
	pkg: PackageIMPL;
	relativePath: string;
}

if (haveNode) {
	var NativeModule: any, vm: any, path: any;
	var process = (globalThis as any).process;
}

const wrapper = [
	'(function (exports, require, module, __filename, __dirname) { ', '\n})'
];

// 'node_modules' character codes reversed
const nmChars = [ 115, 101, 108, 117, 100, 111, 109, 95, 101, 100, 111, 110 ];
const nmLen = nmChars.length;
const CHAR_FORWARD_SLASH = '/'.charCodeAt(0);

function updateChildren(parent: Module, child: Module) {
	var children = parent.children;
	if (!children.includes(child))
		children.push(child);
}

export class Module implements NodeModule {
	id: string;
	exports: any = {};
	parent: Module | null;
	package: Package | null;
	filename: string = ''; // raw filename
	dirname: string = ''; // raw dirname
	loaded: boolean = false;
	children: Module[] = [];
	paths: string[] = [];

	constructor(id: string, parent?: Module, pkg?: Package) {
		this.id = id;
		this.parent = parent || null;
		this.package = pkg || null;
		if (parent) {
			parent.children.push(this);
		}
	}

	// Given a file name, pass it to the proper extension handler.
	load(resolveFilename: string, filename: string) {
		debug('load %j for module %j', resolveFilename, this.id);

		assert(!this.loaded);
		this.filename = filename;
		this.dirname = _path.dirname(filename);

		if (isNetwork(filename)) {
			if (this.package) {
				this.paths = Module._nodeModulePaths(this.dirname, this.package.path);
			}
		} else {
			this.paths = Module._nodeModulePaths(this.dirname);
		}

		var extension = _path.extname(filename) || '.js';
		if (!Module._extensions[extension])
			extension = '.js';
		Module._extensions[extension](this, resolveFilename);
		this.loaded = true;
	}

	require(path: string): any {
		assert(path, 'missing path');
		assert(typeof path === 'string', 'path must be a string');
		return Module._load(path, this, /* isMain */ false);
	}

	// Run the file contents in the correct scope or sandbox. Expose
	// the correct helper variables (require, module, exports) to
	// the file.
	// Returns exception, if any.
	private _compile(content: string, resolveFilename: string) {
		content = stripShebang(content);

		var filename = this.filename;

		// create wrapper function
		var wrapper = Module.wrap(content);

		if (haveNode) {
			var compiledWrapper = vm.runInThisContext(wrapper, {
				filename: filename,// + '#->' + resolveFilename,
				lineOffset: 0,
				displayErrors: true
			});

			var inspectorWrapper = null;
			// needed for setting breakpoint when called with --inspect-brk
			if (process._breakFirstLine && process._eval == null) {
				// Set breakpoint on module start
				if (resolveFilename == 'repl' || this.id == '.') {
					delete process._breakFirstLine;
					inspectorWrapper = process.binding('inspector').callAndPauseOnStart;
					if (!inspectorWrapper) {
						const Debug = vm.runInDebugContext('Debug');
						Debug.setBreakPoint(compiledWrapper, 0, 0);
					}
				}
			}

			filename = fallbackPath(filename);
			var dirname = path.dirname(filename);
			var require = makeRequireFunction(this, mainModule);
			var result;
			if (inspectorWrapper) {
				result = inspectorWrapper(compiledWrapper, this.exports, this.exports, require, this, filename, dirname);
			} else {
				result = compiledWrapper.call(this.exports, this.exports, require, this, filename, dirname);
			}
		} else {
			var wrapper = Module.wrap(content);
			var compiledWrapper = _util.runScript(wrapper, filename/* + '#->' + resolveFilename*/);
			filename = fallbackPath(filename);
			var dirname = _path.dirname(filename);
			var require = makeRequireFunction(this, mainModule);
			var result = compiledWrapper.call(this.exports, this.exports, require, this, filename, dirname);
		}
		return result;
	}

	private static _findPath(request: string, paths: string[], isMain?: boolean) {
		return false;
	}

	private static _nodeModulePaths(from: string, begin?: string) {
		return levelPaths(resolve(from), begin).map(e=>e+'/node_modules');
	}

	private static _resolveLookupPaths(request: string, parent?: Module): string[] | null {
		// Check for node modules paths.
		if (request.charAt(0) !== '.' || // mod or .mode or file:/// or http:/// or / or d:/
				(request.length > 1 &&
				request.charAt(1) !== '.' &&
				request.charAt(1) !== '/' &&
				(!isWindows || request.charAt(1) !== '\\'))) {

			let paths = modulePaths;
			if (parent != null && parent.paths && parent.paths.length) {
				paths = parent.paths.concat(paths);
			}

			debug('Module._resolveLookupPaths-1 for %j in %j', request, paths);
			return paths.length > 0 ? paths : null;
		}

		// ./ or ..

		// In REPL, parent.filename is null.
		if (!parent || !parent.id || !parent.filename) {
			// Make require('./path/to/foo') work - normally the path is taken
			// from realpath(__filename) but in REPL there is no filename
			const mainPaths = ['.'];
	
			debug('Module._resolveLookupPaths-2 for %j in %j', request, mainPaths);
			return mainPaths;
		}

		debug('RELATIVE: requested: %s from parent.id %s', request, parent.id);

		const parentDir = [parent.dirname];
		debug('Module._resolveLookupPaths-3 for %j', parentDir);
		return parentDir;
	}

	private static _resolveFilename(request: string, parent?: Module, isMain?: boolean, options?: { paths?: string[]; }) {
		if (haveNode && NativeModule.nonInternalExists(request)) {
			return request;
		}
		return resolveFilename(request, parent).resolve;
	}

	private static _load(request: string, parent?: Module, isMain?: boolean): any {
		if (parent) {
			debug('Module._load REQUEST %s parent: %s', request, parent.id);
		}

		if (haveNode && NativeModule.nonInternalExists(request)) {
			debug('load native module %j', request);
			return NativeModule.require(request);
		}

		var { filename, resolve, pkg } = resolveFilename(request, parent);

		var cachedModule = Module._cache[filename];
		if (cachedModule) {
			if (parent)
				updateChildren(parent, cachedModule);
			return cachedModule.exports;
		}

		var module = new Module(filename, parent, pkg && pkg.host);

		if (isMain) {
			assert(!mainModule);
			if (haveNode)
				process.mainModule = module;
			module.id = '.';
			mainModule = module;
		}

		Module._cache[filename] = module;

		tryModuleLoad(module, resolve, filename);

		return module.exports;
	}

	private static runMain() {
		delete (Module as any).runMain;
		// Load the main module--the command line argument.

		(async ()=>{ // startup

			var addModulePathWithoutErr = async (path: string)=>{
				try {
					if ( !modulePathCache.hasOwnProperty(path) ) {
						var mp = new ModulePath(path);
						await mp.load();
						if ( !modulePathCache.hasOwnProperty(path) ) {
							modulePaths.unshift(mp.path); // add First
							modulePathCache.set(mp.path, mp);
						}
					}
				} catch(err) {
					print_warn(err);
				}
			};
			await addModulePathWithoutErr(_path.resources());

			var main = _pkgutil.mainPath as string;
			if (main) {
				if (isNetwork(main)) { // 这是一个网络启动,添加一些默认搜索路径
					if (_debug) {
						// load `project/node_modules`
						// add http://127.0.0.1:1026/node_modules to global search path
						var mat = main.match(/^https?:\/\/[^\/]+/) as RegExpMatchArray;
						await addModulePathWithoutErr(mat[0] + '/node_modules');
					}
					// pkg http://127.0.0.1:1026/aaa/bbb/node_modules/pkg
					// add http://127.0.0.1:1026/aaa/bbb/node_modules to global search path
					await addModulePathWithoutErr(resolve(main, '..'));
				}

				//
				if (existsSync(main)) {
					main = resolve(main);
				}
				var _lookup = lookup(main);
				if (_lookup) {
					await _lookup.pkg.host.install();
				}
				Module._load(main, undefined, true);
			}
		})().catch((err: Error)=>{
			if (haveNode) {
				console.error(err.stack || err.message);
				process.exit(-20045); // ERR_UNHANDLED_REJECTION
			} else {
				throw err;
			}
		});

		if (haveNode)
			// Handle any nextTicks added in the first tick of the program
			process._tickCallback();
	}

	static _initPaths() {
		if (!haveNode)
			return;

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
			prefixDir = resolve(process.execPath, '..');
		} else {
			prefixDir = resolve(process.execPath, '..', '..');
		}
		var paths: string[] = [resolve(prefixDir, 'lib', 'node')];

		if (homeDir) {
			paths.unshift(resolve(homeDir, '.node_libraries'));
			paths.unshift(resolve(homeDir, '.node_modules'));
		}

		var nodePath = process.env['NODE_PATH'];
		if (nodePath) {
			paths = nodePath.split(_pkgutil.delimiter).filter(function(path: string) {
				return !!path;
			}).concat(paths);
		}

		for (var path of paths) {
			try {
				if (!modulePathCache.hasOwnProperty(path)) {
					var mp = new ModulePath(path);
					mp.loadSync();
					modulePaths.push(mp.path);
					modulePathCache.set(mp.path, mp);
				}
			} catch(err) {}
		}
	}

	static wrap(script: string) {
		return wrapper[0] + script + wrapper[1];
	}

	static _preloadModules(requests: string[]) {
		if (!Array.isArray(requests))
			return;

		// Preloaded modules have a dummy parent module which is deemed to exist
		// in the current working directory. This seeds the search path for
		// preloaded modules.
		var parent = new Module('internal/preload');
		try {
			parent.paths = Module._nodeModulePaths(_path.cwd());
		} catch (e: any) {
			if (e.code !== 'ENOENT') {
				throw e;
			}
		}
		for (var n = 0; n < requests.length; n++)
			parent.require(requests[n]);
	}

	private static _initNode(nm: any) {
		delete (Module as any)._initNode;
		if (haveNode) {
			NativeModule = nm;
			vm = NativeModule.require('vm');
			path = NativeModule.require('path');

			Object.keys(NativeModule._source)
				.filter(NativeModule.nonInternalExists)
				.forEach(e=>Module.builtinModules.push(e));
			Object.freeze(Module.builtinModules);
		}
	}

	static get globalPaths() {
		return modulePaths.slice();
	}
	static readonly builtinModules: string[] = [];
	static readonly wrapper = wrapper;
	static readonly _cache: Dict<Module> = {};
	static readonly _pathCache: Dict<string> = {};
	static readonly _debug = _pkgutil.debugLog('module');

	static readonly _extensions: Dict<(m: NodeModule, filename: string) => any> = {
		'.js': function(module: NodeModule, filename: string) {
			var content = readTextSync(filename);
			(module as Module)._compile(stripBOM(content), filename);
		},
		'.json': function(module: NodeModule, filename: string) {
			module.exports = readJSONSync(filename);
		},
		'.node': haveNode ? function(module: NodeModule, filename: string) {
			return process.dlopen(module, path._makeLong(fallbackPath(filename)));
		}: function() {
			throw new Error('unrealized');
		},
	};
}

function tryModuleLoad(module: Module, filename: string, rawPathname: string) {
	var threw = true;
	try {
		module.load(filename, rawPathname);
		threw = false;
	} finally {
		if (threw) {
			delete Module._cache[filename];
		}
	}
}

function resolveFilename(request: string, parent?: Module, nocache?: boolean): { filename: string, resolve: string, pkg?: PackageIMPL } {
	var _lookup = lookup(request, parent, nocache);
	if (_lookup) {
		var rr = _lookup.pkg.resolveRelative(_lookup.relativePath);
		return { 
			filename: _lookup.pkg.path + '/' + rr.pathname, 
			resolve: rr.resolve,
			pkg: _lookup.pkg,
		};
	} else {
		var filename = resolve(request);
		var resolveFilename = filename;
		if (isLocal(filename)) {
			if (!isFileSync(filename)) {
				if (!_path.extname(filename)) {
					filename += '.js';
					if (isFileSync(filename)) {
						return { filename, resolve: filename };
					}
				}
				throw_MODULE_NOT_FOUND(filename, parent);
			}
		} else { // no package network file
			resolveFilename = set_url_args(filename, nocache ? '__no_cache': '');
		}
		return {
			filename, resolve: resolveFilename,
		};
	}
}

function getOrigin(from: string) {
	var origin: string = '';

	if (!from)
		return '';

	if (isNetwork(from)) {
		var index = from.indexOf('/', from[4] == 's' ? 9: 8);
		if (index == -1) {
			return from;
		} else {
			return from.substring(0, index);
		}
	} else if (isLocalZip(from)) {
		// zip:///home/xxx/test.apk@/assets/bb.jpg
		origin = from.substring(0, from.indexOf('@/') + 1);
		if (!origin) {
			if (from[from.length - 1] == '@') {
				return from;
			}
		}
	} else {
		var mat = from.match(/^file:\/\/(\/[a-z]:\/)?/i);
		origin = mat ? mat[0]: '';
	}

	return origin;
}

function levelPaths(from: string, begin?: string): string[] {
	// Guarantee that 'from' is absolute.
	// from = resolve(from);

	var prefix = begin ? begin: getOrigin(from);

	if (!prefix) // invalid origin
		return [];

	from = from.substring(prefix.length);

	// Return early not only to avoid unnecessary work, but to *avoid* returning
	// an array of two items for a root: [ '//node_modules', '/node_modules' ]
	if (from === '/')
		return [prefix];

	// note: this approach *only* works when the path is guaranteed
	// to be absolute.  Doing a fully-edge-case-correct path.split
	// that works on both Windows and Posix is non-trivial.
	const paths = [];
	for (let i = from.length - 1, p = 0, last = from.length; i >= 0; --i) {
		const code = from.charCodeAt(i);
		if (code === CHAR_FORWARD_SLASH) {
			if (p !== nmLen)
				paths.push(from.slice(0, last));
			last = i;
			p = 0;
		} else if (p !== -1) {
			if (nmChars[p] === code) {
				++p;
			} else {
				p = -1;
			}
		}
	}

	// Append /node_modules to handle root paths.
	paths.push('');

	return paths.map(e=>prefix+e);
}

function slicePackageName(request: string) {
	var pkgName = request;
	var relativePath = '';
	var index = request.indexOf('/');
	if (index != -1) {
		pkgName = request.substring(0, index);
		relativePath = request.substring(index + 1);
	}
	return {pkgName,relativePath};
}

function lookupFromExtend(pkgName: string, relativePath: string): LookupResult | null {
	// extend pkg
	var pkg = packages.get('ext://' + pkgName);
	if (pkg) {
		var r = { pkg, relativePath };
		lookupCaches.set('ext://' + pkgName + (relativePath ? '/' + relativePath : ''), r);
		return r;
	}
	return null;
}

function lookupFromAbsolute(request: string, lazy?: boolean): LookupResult | null {
	request = resolve(request);

	var cached = lookupCaches.get(request);
	if (cached)
		return cached;

	if (request.substring(0, 4) == 'ext:') {
		var {pkgName,relativePath} = slicePackageName(request.substring(6));
		return lookupFromExtend(pkgName, relativePath);
	}

	var paths = levelPaths(request);
	paths.pop();

	if (paths.length === 0) { // file:///aaa -> aaa module, file:/// -> No directory
		return null;
	}
	var is_local = isLocal(request);

	for (var pkgPath of paths) {
		var pkg = packages.get(pkgPath);
		if (!pkg) {
			if (is_local) {
				if (isDirectorySync(pkgPath)) {
					var json_path = pkgPath + '/package.json';
					if (isFileSync(json_path)) {
						pkg = new PackageIMPL(pkgPath, readJSONSync(json_path));
					}
				} else { // try package link
					var link = readLocalPackageLinkSync(pkgPath + '.link');
					if (link) {
						pkg = packages.get(link);
						if (!pkg)
							pkg = new PackageIMPL(link, readJSONSync(link + '/package.json'));
						packages.set(pkgPath, pkg);
					}
				}
			} else { // network
				var searchPath = pkgPath.substring(0, pkgPath.lastIndexOf('/'));
				var modulePath = modulePathCache.get(searchPath);
				if (modulePath) {
					var pkgName = pkgPath.substring(searchPath.length + 1);
					if (modulePath.hasPackage(pkgName)) {
						pkg = modulePath.createPackage(pkgName);
					}
				}
			}
		}

		if (pkg) {
			var relativePath = request.substring(pkgPath.length + 1);
			if (!is_local && !lazy) { // network
				// TODO network, if relativePath = 'node_modules/pkgName/xxx.js' then ?
				var index = relativePath.indexOf('node_modules');
				if (index != -1) {
					var searchPath = pkg.path + '/' + relativePath.substring(0, index + 12);
					var modulePath = modulePathCache.get(searchPath);
					if (!modulePath) {
						modulePath = new ModulePath(searchPath);
						var ok = modulePath.loadSyncWithoutErr();
						modulePathCache.set(searchPath, modulePath);
						if (ok) { // relookup
							return lookupFromAbsolute(request);
						}
					}
				}
			}
			var r = { pkg, relativePath };
			lookupCaches.set(request, r);
			return r;
		}
	}

	return null;
}

function lookupFromPackage(request: string, parent?: Module): LookupResult | null {
	var paths = modulePaths;
	if (parent && parent.paths && parent.paths.length) {
		paths = parent.paths.concat(paths);
	}

	debug('lookupFromPackage pkg for %j in %j', request, paths);

	var {pkgName,relativePath} = slicePackageName(request);

	for (var searchPath of paths) {
		var pkg = packages.get(searchPath + '/' + pkgName);
		if (!pkg) {
			var modulePath = modulePathCache.get(searchPath);
			if (!modulePath) {
				modulePath = new ModulePath(searchPath);
				modulePath.loadSyncWithoutErr();
				modulePathCache.set(searchPath, modulePath);
			}
			if (modulePath.hasPackage(pkgName)) {
				pkg = modulePath.createPackage(pkgName);
			}
		}
		if (pkg)
			return { pkg, relativePath };
	}

	return lookupFromExtend(pkgName, relativePath);
}

function lookup(request: string, parent?: Module, lazy?: boolean): LookupResult | null {

	// Check for node modules paths.
	if (request.charAt(0) !== '.' || // mod or .mode or file:/// or http:/// or / or d:/
			(request.length > 1 &&
			request.charAt(1) !== '.' &&
			request.charAt(1) !== '/' &&
			(!isWindows || request.charAt(1) !== '\\'))) {

		if (isAbsolute(request)) { // file:/// or http:/// or / or d:/
			return lookupFromAbsolute(request, lazy);
		} else if (!lazy /*no lazy*/) { // mod or .mode
			return lookupFromPackage(request, parent);
		}
	} else { // ./ or ..
		if (parent) { // parent
			return lookupFromAbsolute(parent.dirname + '/' + request);
		} else { // cwd()
			return lookupFromAbsolute(request);
		}
	}

	return null;
}

class Exports {

	get mainPackage() {
		return mainModule && mainModule.package;
	}

	get mainModule() {
		return mainModule;
	}

	get mainFilename() {
		return mainModule ? mainModule.filename: '';
	}

	async addModulePath(path: string) {
		if ( !modulePathCache.hasOwnProperty(path) ) {
			var mp = new ModulePath(path);
			await mp.load();
			if ( !modulePathCache.hasOwnProperty(path) ) {
				modulePaths.push(mp.path);
				modulePathCache.set(mp.path, mp);
			}
		}
	}

	lookupPackage(name: string, parent?: Module): Package | null {
		var _lookup = lookup(name, parent);
		return _lookup ? _lookup.pkg.host: null;
	}

}

enum PackageStatus {
	NO_INSTALL = -1,
	INSTALLED = 0,
	INSTALLING = 1,
}

class PackageIMPL {
	host: Package;
	json: PackageJson;
	name: string; // package 名称
	path: string; // package 路径, zip:///applications/test.apk@
	hash: string;
	build: boolean;
	status: PackageStatus = PackageStatus.NO_INSTALL;
	versions: Dict<string> = {}; // .pkg 包内文件的版本信息
	pkg_files: Set<string> = new Set();
	pkg_path: string = ''; // zip:///root/aa/aa.pkg@
	path_cache: Map<string, [string/*pathname*/,string/*resolve*/]> = new Map();
	helper?: PackageIMPL; // local helper package
	helperAll = false; // `this.hash === helper.hash` 完全使用 helper
	isNetwork: boolean;

	constructor(path: string, json: PackageJson) {
		this.json = json;
		this.name = json.name;
		this.path = path;
		this.hash = json.hash || '';
		this.build = !!json.hash;
		this.isNetwork = isNetwork(path);

		var pathname = _path.basename(path);

		// assert(json.name == name, `Lib name must be consistent with the folder name, ${json.name} != ${name}`);
		assert(!packages.has(path), `${path} package repeat create`);
		packages.set(path, this);

		this.host = new Package(this); // create host

		if (this.isNetwork && this.build) {
			// query helper package from global modulePaths
			for (var path of modulePaths) {
				var mp = modulePathCache.get(path) as ModulePath;
				if (mp.hasPackage(pathname) && !mp.isNetwork(pathname)) {
					var json = mp.packageJson(pathname);
					if (json.hash) { // is build
						this.helper = mp.createPackage(pathname);
						break;
					}
				}
			}
		}
	}

	private _resolveRelativeAfter(key: string, pathname: string, version: string) {
		var self = this;
		var resolve: string = '';
	
		if (self.helper) { // 读取本地旧文件
			var helper = self.helper;
			// 版本相同,完全可以使用本地旧文件路径,这样可以避免从网络下载新资源
			if ( helper.versions[pathname] === version ) {
				if ( helper.pkg_files.has(pathname) ) {
					resolve = helper.pkg_path + '/' + pathname;
				} else {
					resolve = helper.path + '/' + pathname;
				}
			}
		}
	
		if (!resolve) {
			if ( self.pkg_path && self.pkg_files.has(pathname) ) { // 使用.pkg
				resolve = self.pkg_path + '/' + pathname;
			} else {
				resolve = self.path + '/' + pathname;
			}
		}
	
		resolve = set_url_args(resolve, version);
	
		self.path_cache.set(key, [pathname, resolve]);
	
		return { pathname, resolve };
	}

	resolveRelative(relativePath: string): { pathname: string, resolve: string } {
		var self = this; 
		self.installSync();

		if (self.helperAll)
			return (self.helper as PackageIMPL).resolveRelative(relativePath);
	
		var pathname = relativePath;
		var cached = self.path_cache.get(pathname);
		if (cached) {
			return { pathname: cached[0], resolve: cached[1] };
		}

		if (!pathname) {
			pathname = std_package_main(self.json.main);
		}

		var ver: string | undefined, file_pathnames: string[];
	
		if (_path.extname(pathname)) {
			ver = self.versions[pathname];
			if ( ver === undefined ) { // 找不到版本信息
				if (!this.isNetwork) { // local
					var src = self.path + '/' + pathname;
					if (isFileSync(src)) { // 尝试访问本地文件系统,是否能找到文件信息
						return self._resolveRelativeAfter(relativePath, pathname, '');
					}
				}
				file_pathnames = [pathname + '/index']; // 尝试做为目录使用
			} else {
				return self._resolveRelativeAfter(relativePath, pathname, ver);
			}
		} else {
			// 没有扩展名,尝试使用多个扩展名查找 .js .json .node ...
			file_pathnames = pathname ? [pathname, pathname + '/index']: ['index'];
		}

		var extnames = Object.keys(Module._extensions);

		// 尝试使用尝试默认扩展名不同的扩展名查找, and `${pathname}/index`
		for (var pathname of file_pathnames) {
			for (var ext of extnames) {
				ver = self.versions[pathname + ext];
				if (ver !== undefined) {
					return self._resolveRelativeAfter(relativePath, pathname + ext, ver);
				}
			}
			if (!this.isNetwork) { // 尝试访问本地文件系统,是否能找到文件信息
				for (var ext of extnames) {
					var src = self.path + '/' + pathname + ext;
					if ( isFileSync(src) ) { // find local
						return self._resolveRelativeAfter(relativePath, pathname + ext, '');
					}
				}
			}
		}

		throw_MODULE_NOT_FOUND(self.path + '/' + relativePath);
		throw '';
	}

	private _installComplete(versions_path: string, cb?: Cb) {
		var self = this;

		if (self.build || isNetwork(versions_path)) {
			// 读取package内部资源文件版本信息
			var versions_json = versions_path + '/versions.json';
			// 这里如果非build状态,不使用缓存
			var versions_json_arg = set_url_args(versions_json, self.hash || '__no_cache');
	
			var read_versions_ok = function(str: string) {
				var data = _parseJSON(str, versions_json);
				self.versions = data.versions || {};
				self.status = PackageStatus.INSTALLED;
				if (self.pkg_path) {
					for (var [file, ver] of Object.entries(self.versions)) {
						if (ver.charCodeAt(0) != 46 /*.*/)
							self.pkg_files.add(file); // .pkg 中包含的文件列表 
					}
				}
				cb && cb(); // ok
			};
	
			if (cb)
				readText(versions_json_arg).then(read_versions_ok).catch(cb);
			else
				read_versions_ok(readTextSync(versions_json_arg));
		} else {
			self.status = PackageStatus.INSTALLED;
			cb && cb(); // ok
		}
	}
	
	private _installNetwork(cb?: Cb) {
		var self = this;
		if (self.helper) {
			if (self.hash == self.helper.hash) { // 完全相同的两个包
				self.helperAll = true;
				self.status = PackageStatus.INSTALLED;
				return cb && cb();
			}
		}
	
		// 如果本地不存在相应版本的文件,下载远程.pkg文件到本地
		// 远程.pkg文件必须存在否则抛出异常
		var hash = self.hash;
		var path = _path.temp(`${self.name}.pkg`);
		var pathname = `${path}.${hash}`;
	
		// zip:///Users/pppp/sasa/aa.apk@/aaaaa/bbbb/aa.js
	
		if (_fs.existsSync(pathname)) { // 文件存在,无需下载
			// 设置一个本地zip文件读取协议路径,使用这种路径可直接读取zip内部文件
			self.pkg_path = `zip:///${pathname.substring(8)}@`;  // file:///
			self._installComplete(self.pkg_path, cb);
		} else { // downloading ...
			var url = set_url_args(`${self.path}/${self.name}.pkg`, hash);
			var save = pathname + '.~';
	
			var tryOld = function() {
				// TODO ... Try to query the old package
				// TODO ...
				if (self.helper) { // use helper pkg
					self.helperAll = true;
					self.status = PackageStatus.INSTALLED;
					cb && cb();
					return true;
				}
			};
	
			// TODO 文件比较大时需要断点续传下载
			// TODO 还应该使用读取数据流方式,实时回调通知下载进度
			var ok = function(err?: Error) { // 下载成功
				if (err) {
					if (!tryOld()) {
						throwErr(err, cb);
					}
				} else {
					_fs.renameSync(save, pathname);
					self.pkg_path = `zip:///${pathname.substring(8)}@`; // file:///
					self._installComplete(self.pkg_path, cb);
				}
			};
	
			if (cb) {
				_http.request({ url, save }, ok);
			} else {
				try {
					_http.requestSync({ url, save });
				} catch(err: any) {
					ok(err); return;
				}
				ok();
			}
		}
	}

	install(cb?: Cb) {
		var self = this;
		var path = self.path;
	
		if (self.status != PackageStatus.NO_INSTALL) {
			return throwErr(`${path} package installing repeat call`, cb);
		}
		self.status = PackageStatus.INSTALLING;
	
		if (!self.build) { // Not build
			return self._installComplete(path, cb);
		}
	
		if (self.isNetwork) { // network
			return self._installNetwork(cb);
		}
	
		// install local
	
		/*
		* build的pkg有两种格式
		* 1.pkg根目录存在.pkg压缩文件,文件中包含全部文件版本信息与一部分源码文件以及资源文件.
		* 2.pkg根目录不存在.pkg压缩文件,相比build前只多出文件版本信息,适用于android/ios安装包中存在.
		*/
		/* 文件读取器不能读取zip包内的.pkg压缩文件.
		* 比如无法android资源包中的.pkg文件
		* 所以android.apk中不能存在.pkg格式文件否则将不能读取
		*/

		if (isLocalZip(path)) { // 包路径在zip包中
			self._installComplete(path, cb);
		}
		else if (isFileSync(`${path}/${self.name}.pkg`)) { // 本地包中存在.pkg文件
			self.pkg_path = `zip:///${path.substring(8)}/${self.name}.pkg@`;  // file:///
			self._installComplete(self.pkg_path, cb);
		}
		else { // 无.pkg包
			self._installComplete(path, cb);
		}
	}

	installSync() {
		var self = this;
		if (self.status !== 0) {
			if (self.helper) // install helper
				self.helper.installSync();
			try {
				self.install();
			} catch(err) {
				self.status = PackageStatus.NO_INSTALL;
				throw err;
			}
		}
	}

}

/**
 * ext://pkg ExtendModule Internal expansion module
 * 
 * @class PackageExtend
 */
class PackageExtend extends PackageIMPL {

	constructor(name: string, modules: string[]) {
		super('ext://' + name, {
			name,
			main: 'index.js',
			version: '0.0.0',
		});
		this.status = PackageStatus.INSTALLED;
		modules.forEach(e=>this.versions[e]='-');
	}

	resolveRelative(relativePath: string): { pathname: string, resolve: string } {
		var self = this;

		var pathname = relativePath;
		var cached = self.path_cache.get(pathname);
		if (cached) {
			return { pathname: cached[0], resolve: cached[1] };
		}

		if (!pathname) {
			// It must be a standard
			pathname = self.json.main; // std_package_main(self.json.main);
		}

		if (!this.versions.hasOwnProperty(pathname)) {
			var paths = Object.keys(Module._extensions).map(e=>pathname + e);
			pathname = '';
			for (var path of paths) {
				if (this.versions.hasOwnProperty(path)) {
					pathname = path; break;
				}
			}
			if (!pathname)
				throw_MODULE_NOT_FOUND(self.path + '/' + relativePath);
		}

		var resolve = this.path + '/' + pathname;

		self.path_cache.set(relativePath, [pathname, resolve]);

		return { pathname, resolve };
	}

	static _init() {
		interface ExtendModule { 
			filename: string;
			extname: string;
		}
		var tmps: Dict<string[]> = {};
		for ( var [, v] of Object.entries<ExtendModule>(_util.__extendModule) ) {
			var i = v.filename.indexOf('/');
			if (i > 0) { // 没有目录的扩展包被忽略  quark/value
				var mname = v.filename.substring(0, i);
				var m = tmps[mname];
				if (!m) {
					tmps[mname] = m = [];
				}
				m.push(v.filename.substring(i + 1));
			}
		}

		for ( var [name, files] of Object.entries(tmps)) {
			new PackageExtend(name, files);
		}
	}
}

class Package {
	private _impl: PackageIMPL;

	get json() { return this._impl.json }
	get name() { return this._impl.name }
	get path() { return this._impl.path }

	constructor(impl: any) {
		this._impl = impl;
	}

	resolve(relativePath: string) {
		return this._impl.resolveRelative(relativePath).resolve;
	}

	async install() {
		var self = this._impl;
		if (self.status !== 0) {
			if (self.helper) // install helper
				await self.helper.host.install();
			await new Promise<void>(function(resolve, reject) {
				self.install(function(err) {
					if (err) {
						self.status = PackageStatus.NO_INSTALL;
						reject(err);
					}
					else resolve();
				});
			});
		}
	}
}

PackageExtend._init();
Module._initPaths();

export default new Exports();