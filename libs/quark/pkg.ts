/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

__binding__('_ext');
const _init = __binding__('_init');
const _fs = __binding__('_fs');
const _http = __binding__('_http');
const _util = __binding__('_util');
const { formatPath, isAbsolute, isLocal, isLocalZip, stripShebang,
				isHttp, assert, stripBOM, default: _utild } = _util;
const { readFile, readFileSync, isFileSync,
				isDirectorySync, readdirSync } = _fs.reader;
const debug = _util.debugLog('PKG');
const win32 = _utild.platform == 'win32';

/// <reference path="./_ext.d.ts" />

type Optopns = Dict<string>;

interface Cb {
	(err?: Error): void;
}

function print(...args: any[]) {
	console.warn(...args.map(e=>(e instanceof Error ? e.message: e)));
}

function throwErr(e: any, cb?: Cb) {
	if (! (e instanceof Error)) {
		e = e && typeof e == 'object' ?
			Object.assign(new Error(e.message || e.error || 'Unknown error'), e): new Error(e);
	}
	let err = e as Error;
	if (!err.errno)
		err.errno = -30000;
	if (cb)
		cb(err);
	else
		throw err;
}

function throwModuleNotFound(request: string, parent?: Module) {
	let msg = `Cannot find module '${request}'`;
	if (parent)
		msg += ` in module ${parent.filename}`;
	let err = new Error(msg);
	err.errno = -12000; // ERR_MODULE_NOT_FOUND
	throw err;
}

function set_url_args(path: string, arg?: string) {
	if (/^(https?):\/\//i.test(path)) {
		let url_arg = arg || options.url_arg;
		if ( url_arg )
			return path + (path.indexOf('?') == -1 ? '?' : '&') + url_arg;
	}
	return path;
}

function parseJSON(source: string, filename: string) {
	try {
		return JSON.parse(stripBOM(source));
	} catch (err: any) {
		err.message = filename + ': ' + err.message;
		throw err;
	}
}

const readText     = (path: string)=>new Promise<string>(function(resolve, reject) {
	readFile((err?: Error, r?: any)=>err?reject(err):resolve(r), path, 'utf8')
});
const readTextSync = (path: string)=>readFileSync(path, 'utf8') as string;
const readJSON     = async (filename: string)=>parseJSON(await readText(filename), filename);
const readJSONSync = (filename: string)=>parseJSON(readTextSync(filename), filename);

function readLocalPackageLinkSync(pathname: string) {
	if (isFileSync(pathname)) {
		let link = readTextSync(pathname);
		link = isAbsolute(link) ? formatPath(link): formatPath(pathname, '..', link);
		if (isDirectorySync(link)) {
			if ( isFileSync(link + '/package.json') ) {
				return link;
			}
		}
	}
}

interface LookupResult {
	pkg: Package;
	relativePath: string;
}
const saerchModules = 'node_modules'; // search modules directory, using `node_maodules`
// global search paths list
const globalPaths: string[] = []; // /node_modules, /test/node_modules
// /node_modules => SearchPath or /test/node_modules => SearchPath
const searchPaths: Map<string, SearchPath> = new Map(); // absolute path => SearchPath
// qk://quark/index => LookupResult
const lookupCache: Map<string, LookupResult> = new Map(); // absolute path => LookupResult
// all packages
const packages: Map<string, Package> = new Map();
const options: Optopns = _utild.options;
let   mainModule: Module | undefined;

const wrapper = [
	'(function (exports, require, module, __filename, __dirname) { ', '\n})'
];

function updateChildren(self: Module, child: Module) {
	let children = self.children;
	if (!children.includes(child))
		children.push(child);
}

function resolveFilename(request: string, parent?: Module): { filename: string, resolve: string, pkg?: Package } {
	let _lookup = lookup(request, parent);
	if (_lookup) {
		let {resolve,pathname} = _lookup.pkg.resolve(_lookup.relativePath);
		return { 
			filename: _lookup.pkg.path + '/' + pathname,
			resolve: resolve,
			pkg: _lookup.pkg,
		};
	} else {
		let filename = formatPath(request), resolve = '';
		if (isLocal(filename)) { // file:/// or zip:/// or / or d:/
			for (let ext of ['', ...Object.keys(Module._extensions)]) {
				resolve = filename + ext;
				if (isFileSync(resolve))
					return { filename, resolve };
			}
			throwModuleNotFound(filename, parent); // throw error
		} else { // no package network file
			resolve = set_url_args(filename);
		}
		return {
			filename, resolve,
		};
	}
}

/**
 * http://google.com/aa/test.txt => http://google.com
 * zip:///home/xxx/test.apk@/assets/bb.jpg => zip:///home/xxx/test.apk@
 * file:///d:/a/bc/d/e/f/test.txt => file://d:
 * file:///a/bc/d/e/f/test.txt => file://
*/
function getOrigin(from: string): string {
	let origin: string = '';

	if (!from)
		return '';

	if (isHttp(from)) {
		let index = from.indexOf('/', from[4] == 's' ? 9: 8);
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
		let mat = from.match(/^file:\/\/(\/[a-z]:)?/i);
		origin = mat ? mat[0]: '';
	}

	return origin;
}

function parentsPaths(from: string, prefix?: string): string[] {
	// Guarantee that 'from' is absolute.

	// prefix => file://d:, file://, http://google.com, zip:///home/xxx/test.apk@
	prefix = prefix || getOrigin(from);

	if (!prefix) // invalid origin
		return [];

	from = from.substring(prefix.length);

	// Return early not only to avoid unnecessary work, but to *avoid* returning
	// an array of two items for a root: [ '//node_modules', '/node_modules' ]
	if (from === '/')
		return [prefix];

	// Append /node_modules to handle root paths.
	const paths = [prefix];

	for (let s of from.split('/').slice(1)) {
		prefix += '/' + s;
		if (s != saerchModules) {
			paths.push(prefix);
		}
	}
	return paths.reverse();
}

function slicePackageName(request: string) {
	let pkgName = request;
	let relativePath = '';
	let index = request.indexOf('/');
	if (index != -1) {
		pkgName = request.substring(0, index);
		relativePath = request.substring(index + 1);
	}
	return {pkgName,relativePath};
}

function lookupFromAbsolute(request: string): LookupResult | null {
	request = formatPath(request); // Guarantee that 'request' is absolute.

	const cached = lookupCache.get(request);
	if (cached) {
		return cached;
	}

	if (request.substring(0, 5) == 'qk://') {
		const {pkgName,relativePath} = slicePackageName(request.substring(5));
		let pkg = packages.get(`qk://${pkgName}`);
		if (pkg) {
			let r = { pkg, relativePath };
			lookupCache.set(`qk://${pkgName}${relativePath && '/'+relativePath}`, r);
			return r;
		}
		return null;
	}

	const is_local = isLocal(request);
	const paths = parentsPaths(request); // search paths
	if (is_local)
		paths.pop();
	/*
		file:///a/b/c/d
		file:///a/b/c
		file:///a/b
		file:///a
	*/

	if (paths.length === 0) // file:///aaa -> aaa module, file:/// -> No directory
		return null;

	for (let pkgPath of paths) {
		let pkg = packages.get(pkgPath);
		if (!pkg) { // no pkg
			if (is_local) { // local
				if (isDirectorySync(pkgPath)) {
					let jsonPath = pkgPath + '/package.json';
					if (isFileSync(jsonPath))
						pkg = new Package(pkgPath, readJSONSync(jsonPath));
				} else { // try package link
					let link = readLocalPackageLinkSync(pkgPath + '.link');
					if (link) {
						pkg = packages.get(link) ||
							new Package(link, readJSONSync(`${link}/package.json`));
						packages.set(pkgPath, pkg); // set alias
					}
				}
			} else { // network http
				let modules = pkgPath.substring(0, pkgPath.lastIndexOf('/')); // modules dir
				// modules: http://xxxx.com/aa/node_modules/test_mod => http://xxxx.com/aa/node_modules
				let searchPath = searchPaths.get(modules); // is match search
				if (searchPath) {
					let pkgName = pkgPath.substring(modules.length + 1);
					if (searchPath.hasPackage(pkgName)) {
						pkg = searchPath.makePackage(pkgName);
					}
				}
			}
		}
		if (pkg) {
			let result = {
				pkg, relativePath: request.substring(pkgPath.length + 1),
			};
			lookupCache.set(request, result);
			return result;
		}
	}

	return null;
}

function lookupFromInternal(pkgName: string, relativePath: string): LookupResult | null {
	let pkg = packages.get(`qk://${pkgName}`);
	return pkg ? { pkg, relativePath }: null;
}

/*
 * ```ts
 * const test = require('test_pkg/index')
 * import * as test from 'test_pkg/index'
 * ```
*/
function lookupFromSearch(request: string, parent?: Module): LookupResult | null {
	const {pkgName,relativePath} = slicePackageName(request);
	let pkg = packages.get(`qk://${pkgName}`); // lookup internal pkg
	if (pkg) {
		return { pkg, relativePath };
	}
	let paths = globalPaths; // search path
	if (parent && parent.paths && parent.paths.length) {
		paths = parent.paths.concat(paths);
	}
	// debug('lookupFromSearch pkg for %j in %j', request, paths);

	for (let path of paths) {
		let pkg = packages.get(`${path}/${pkgName}`);
		if (!pkg) {
			let searchPath = searchPaths.get(path);
			if (!searchPath && isLocal(path)) {
				searchPath = new SearchPath(path);
				try {
					if (!searchPath.isHttp && isDirectorySync(searchPath.path))
						searchPath.loadSync();
				} catch(err) {} finally {
					searchPaths.set(path, searchPath); // avoid recreating next time
				}
			}
			if (searchPath && searchPath.hasPackage(pkgName)) {
				pkg = searchPath.makePackage(pkgName);
			}
		}
		if (pkg)
			return { pkg, relativePath };
	}
	return null;//lookupFromInternal(pkgName, relativePath);
}

function lookup(request: string, parent?: Module): LookupResult | null {
	// Check for node modules paths.
	if (request.charAt(0) !== '.' || // mod or file:/// or http:/// or / or d:/
			(request.length > 1 &&
			request.charAt(1) !== '.' &&
			request.charAt(1) !== '/' &&
			(!win32 || request.charAt(1) !== '\\'))
	) {
		if (isAbsolute(request)) { // file:/// or http:/// or / or d:/
			return lookupFromAbsolute(request);
		} else { // search mod
			return lookupFromSearch(request, parent);
		}
	} else { // ./ or ..
		if (parent) {
			// cwd node_modeuls/xxxx/
			// require('../../../../../aa');
			return lookupFromAbsolute(parent.dirname + '/' + request);
		} else { // cwd()
			return lookupFromAbsolute(request);
		}
	}
}

/**
 * @interface PackageJson
*/
export interface PackageJson {
	name: string; //!<
	main: string; //!<
	version: string; //!<
	description?: string; //!<
	scripts?: Dict<string>; //!<
	author?: Dict<string>; //!<
	keywords?: string[]; //!<
	license?: string; //!<
	bugs?: Dict<string>; //!<
	homepage?: string; //!<
	devDependencies?: Dict<string>; //!<
	dependencies?: Dict<string>; //!<
	bin?: string; //!<
	hash?: string; //!<
	pkgzHash?: string; //!<
	id?: string; //!<
	app?: string; //!<
	tryLocal?: boolean; //!<
	symlink?: string; //!< package path symlink
	pkgzSize?: number; //!< pkgz file size
	modules?: Dict<Dict<PackageJson>>; //!<
}

class SearchPath {
	readonly path: string;
	readonly isHttp: boolean;
	private _jsons?: Dict<PackageJson>; // package name => PackageJson
	private _paths = new Map<string, string>(); // package name => package path

	constructor(path: string, jsons?: Dict<PackageJson>) {
		assert(path);
		assert(typeof path == 'string', 'Search Path type error');
		this.path = formatPath(path);
		this.isHttp = isHttp(path);

		if (jsons) {
			this.loadFrom(jsons);
		}
	}

	/**
	 * load packages name and version and files hash
	*/
	async load(noCache?: boolean) {
		if (this.isHttp) {
			let path = set_url_args(`${this.path}/modules.json`, noCache ? '__no_cache': '');
			this.loadFrom(await readJSON(path));
		} else { // local
			this.loadSync();
		}
		return this;
	}

	/**
	 * async load packages name and version and files hash
	*/
	loadSync() {
		if (this.isHttp) {
			let path = set_url_args(`${this.path}/modules.json`); // modules.json
			this.loadFrom(readJSONSync(path));
		}
		else { // local read mode
			assert(isDirectorySync(this.path), `"${this.path}" is not a directory`);
			for (let dirent of readdirSync(this.path) ) {
				if (dirent.type == 2/*dir*/ ||
					(dirent.type == 3/*symlink*/ && isDirectorySync(dirent.pathname))
				) { // directory
					if ( isFileSync(dirent.pathname + '/package.json') ) {
						this._paths.set(dirent.name, dirent.pathname);
					}
				}
				else if (_fs.extname(dirent.pathname) == '.link') { // has package link
					let link = readLocalPackageLinkSync(dirent.pathname);
					if (link) {
						this._paths.set(dirent.name.substring(0, dirent.name.length - 5), link);
					}
				}
			}
			searchPaths.set(this.path, this);
		}
		return this;
	}

	private loadFrom(jsons: Dict<PackageJson>) {
		assert(jsons, 'loadFrom, Packages cannot be empty');
		this._jsons = jsons;
		this._paths.clear();

		for (const [pkgName,json] of Object.entries(jsons)) {
			let path = `${this.path}/${pkgName}`;
			let link = json.symlink as string;
			if (link) { // json.symlink;
				path = isAbsolute(link) ? formatPath(link): formatPath(this.path, link);
			}
			this._paths.set(pkgName, path);

			// children search paths
			let modules = json.modules;
			if (modules) {
				for (let dir in modules) {
					new SearchPath(`${this.path}/${pkgName}/${dir}`, modules[dir]);
				}
			}
		}
		searchPaths.set(this.path, this);
	}

	hasPackage(pkgName: string) {
		return this._paths.has(pkgName);
	}

	testIsHttp(pkgName: string) {
		let path = this._paths.get(pkgName);
		assert(path);
		return isHttp(path);
	}

	getPackageJson(pkgName: string): PackageJson {
		let path = this._paths.get(pkgName);
		assert(path, `package ${pkgName} does not exist in ${this.path} path`);
		if (this._jsons) {
			return this._jsons[pkgName];
		} else { // read
			return readJSONSync(`${path}/package.json`);
		}
	}

	makePackage(pkgName: string) {
		let path = this._paths.get(pkgName)!;
		let pkg = packages.get(path) || new Package(path, this.getPackageJson(pkgName));
		let rawPath = this.path + '/' + pkgName; // set alias
		if (!packages.has(rawPath))
			packages.set(rawPath, pkg);
		return pkg;
	}
}

type IModule = typeof module;

/**
 * @class Module
*/
export class Module implements IModule {
	readonly id: string; //!<
	readonly exports: any = {}; //!<
	readonly parent: Module | null; //!<
	readonly package: Package | null; //!<
	readonly filename: string = ''; //!< filename
	readonly dirname: string = ''; //!< dirname
	readonly loaded: boolean = false; //!<
	readonly children: Module[] = []; //!<
	readonly paths: string[] = []; //!< search paths

	constructor(id: string, parent?: Module, pkg?: Package) {
		this.id = id;
		this.parent = parent || null;
		this.package = pkg || null;
		if (parent) {
			parent.children.push(this);
		}
	}

	// resolve lookup paths, _resolveLookupPaths()
	private static _resolvePaths(request: string, parent?: Module): string[] | null {
		// Check for node modules paths.
		if (request.charAt(0) !== '.' || // mod or .mode or file:/// or http:/// or / or d:/
				(request.length > 1 &&
				request.charAt(1) !== '.' &&
				request.charAt(1) !== '/' &&
				(!win32 || request.charAt(1) !== '\\')))
		{
			let paths = globalPaths;
			if (parent && parent.paths && parent.paths.length) {
				paths = parent.paths.concat(paths);
			}
			debug('_resolvePaths for %j in %j', request, paths);
			return paths.length > 0 ? paths : null;
		}

		// ./ or ..
		// In REPL, parent.filename is null.
		if (!parent || !parent.id || !parent.filename) {
			// Make require('./path/to/foo') work - normally the path is taken
			// from realpath(__filename) but in REPL there is no filename
			const mainPaths = ['.'];
			debug('_resolvePaths 1 for %j in %j', request, mainPaths);
			return mainPaths;
		}

		debug('RELATIVE: requested: %s from parent.id %s', request, parent.id);
		const parentDir = [parent.dirname];
		debug('_resolvePaths 2 for %j', parentDir);
		return parentDir;
	}

	private _makeRequire(main?: Module): qk.Require {
		let self = this;
		function require(path: string) {
			return self.require(path);
		}
		function resolve(request: string) {
			return resolveFilename(request, self).resolve;
		}
		resolve.paths = (request: string)=>Module._resolvePaths(request, self);
		require.resolve = resolve;
		require.main = main;
		require.extensions = Module._extensions; // Enable support to add extra extension types.
		require.cache = Module._cache;
		return require;
	}

	// Run the file contents in the correct scope or sandbox. Expose
	// the correct helper variables (require, module, exports) to
	// the file.
	// Returns exception, if any.
	private _compile(content: string) {
		let filename = this.filename;
		let wrapper = Module.wrap(stripShebang(content));
		let compiledWrapper = _utild.runScript(wrapper, filename);
		let dirname = _fs.dirname(filename);
		let require: qk.Require;

		if (mainModule) {
			require = this._makeRequire(mainModule);
		} else {
			(this as any).id = '.';
			require = this._makeRequire(mainModule = this);
			if ('inspect_brk' in options || 'brk' in options) {
				_init.debuggerBreakNextStatement();
			}
		}
		compiledWrapper.call(this.exports, this.exports, require, this, filename, dirname);
	}

	/**
	 * Request module by the path
	*/
	require(path: string): any {
		assert(path, 'missing path');
		assert(typeof path === 'string', 'path must be a string');
		return Module.load(path, this);
	}

	// Given a file name, pass it to the proper extension handler.
	private _load(resolve: string, filename: string) {
		assert(!this.loaded);

		let self = this as {
			paths: string[], filename: string, dirname: string, loaded: boolean, exports: any
		};
		self.filename = filename;
		self.dirname = _fs.dirname(filename);

		if (!isHttp(filename)) { // no http
			self.paths = parentsPaths(this.dirname).map(e=>e+'/'+saerchModules);
		} else if (this.package) { // http
			self.paths = [this.package.path +'/'+saerchModules];
		}

		let extension = _fs.extname(filename) || '.js';
		if (!Module._extensions[extension])
			extension = '.js';

		if (this.package && this.package.isInternal) { // quark inline module
			let mod = resolve
				.substring(0, resolve.length - extension.length)
				.substring(5);
			self.exports = __binding__(mod);
		} else {
			Module._extensions[extension](this, resolve);
		}
		self.loaded = true;
	}

	static require(request: string) {
		return Module.load(request, mainModule);
	}

	static load(request: string, parent?: Module): any {
		let { filename, resolve, pkg } = resolveFilename(request, parent);
		let cachedModule = Module._cache[filename];
		if (cachedModule) {
			if (parent)
				updateChildren(parent, cachedModule);
			return cachedModule.exports;
		}
		let module = new Module(filename, parent, pkg);

		Module._cache[filename] = module;

		debug('load %j for module %j', resolve, parent && parent.id);

		let threw = true;
		try {
			module._load(resolve, filename);
			threw = false;
		} finally {
			if (threw) {
				delete Module._cache[filename];
			}
		}
		return module.exports;
	}

	private static async tryReadJSONByNet(url: string): Promise<PackageJson> {
		try { return await readJSON(url) } catch(err: any) {
			if (err.errno == -10001 && err.status == 404) throw err;
		}
		try { await readText('https://www.gnu.org/gnu/') } catch(err: any) {}
		let retry = 10;
		do {
			try { return await readJSON(url) } catch(err) {
				if (retry-- == 0)
					throw err;
				await _utild.sleep(1e3);
			}
		} while(true);
	}

	private static async runMain() {
		delete (Module as any).runMain;

		const res = _fs.resources(), cwd = _util.cwd();
		// add cwd/resources path as global search path
		for (let path of [res, cwd]) {
			if (isDirectorySync(`${path}/${saerchModules}`)) {
				globalPaths.push((new SearchPath(`${path}/${saerchModules}`).loadSync()).path);
			}
		}

		// Load the main module--the command line argument.
		let main = _util.mainPath as string;
		if (!main) {
			var e = options.eval || options.e;
			if (e) {
				let module = new Module('eval');
				let self = module as { filename: string, dirname: string };
				self.filename = 'eval';
				self.dirname = '.';
				module._compile(e)
			}
			return; // no main module, exit
		}
		main = formatPath(main); // format path

		if (isHttp(main)) { // this is a http startup, adding some default search paths
			try {
				let idx = main.indexOf('?'), search = '';
				if (idx != -1) {
					main = main.substring(0, idx), search = main.substring(idx);
				}
				let json = await Module.tryReadJSONByNet(`${main}/package.json${search}`);
				let _res = lookup(res) || lookup(cwd); // lookup local main package

				// create http main pkg and local package
				new Package(main, json, _res?.pkg.name == json.name ? _res!.pkg: undefined);

				// add global search path
				let modules = json.modules;
				if (modules) {
					for (let dir in modules) {
						let search = new SearchPath(`${main}/${dir}`, modules[dir]);
						if (dir == saerchModules) {
							globalPaths.unshift(search.path);
						}
					}
				}
			} catch(err) {
				print(err);
			}
		}

		let _main = lookup(main); // lookup main pkg
		if (_main) { // install pkg
			await _main.pkg.install();
		}
		Module.load(main); // load main module
	}

	static wrap(script: string) {
		return wrapper[0] + script + wrapper[1];
	}

	static get globalPaths() {
		return globalPaths.slice();
	}
	static readonly wrapper = wrapper;
	static readonly _cache: Dict<Module> = {};

	static readonly _extensions: Dict<(module: Module, filename: string) => any> = {
		'.js': function(module: Module, filename: string) {
			module._compile(stripBOM(readTextSync(filename)));
		},
		'.json': function(module: Module, filename: string) {
			(module as any).exports = readJSONSync(filename);
		},
	};
}

enum PackageStatus {
	NO_INSTALL = -1,
	INSTALLED = 0,
	INSTALLING = 1,
}

/**
 * @class Package
*/
class Package {
	// @private
	protected _status: PackageStatus = PackageStatus.NO_INSTALL;
	protected _pathCache: Map<string, { pathname: string, resolve: string }> = new Map();
	protected _pkgzFiles: Set<string> = new Set(); // files in .pkgz
	private   _pkgzPath: string = ''; // zip:///root/aa/aa.pkgz@
	private   _local?: Package; // reference local package
	private   _localOnly = false; // `this.hashs.all === this._local.hashs.all` full using local pkg
	// @public
	/** package json information */
	readonly json: PackageJson;
	/** package name */
	readonly name: string;
	/**
	* Did you attempt to use a local reference package when an error 
	* occurred while downloading and installing the network package
	*/
	readonly tryLocal: boolean; //  default true
	/**
	 * package path, zip:///applications/test.apk@, qk://quark, file:///node_modules/xxxx
	 */
	readonly path: string;
	/** package hash */
	readonly hash: string;
	/** pkgz hash */
	readonly pkgzHash: string;
	/** version information of files in the package */
	readonly filesHash: Dict<string> = {};
	/** is build */
	readonly build: boolean;
	/** is http path */
	readonly isHttp: boolean;
	/** is internal package */
	get isInternal(): boolean { return false }

	constructor(path: string, json: PackageJson, local?: Package) {
		this.json = json;
		this.name = json.name;
		this.path = path;
		this.hash = json.hash||'';
		this.pkgzHash = json.pkgzHash||'';
		this.build = !!json.hash;
		this.tryLocal = 'tryLocal' in json ? !!json.tryLocal: true; // default try ref package
		this.isHttp = isHttp(path);

		assert(!packages.has(path), `${path} package repeat create`);
		packages.set(path, this);

		if (this.isHttp && this.build) {
			if (local && local.build) {
				this._local = local;
			} else {
				let name = _fs.basename(path);
				// query ref package from global search paths
				for (let path of globalPaths) {
					let searchPath = searchPaths.get(path)!;
					if (searchPath.hasPackage(name) && !searchPath.testIsHttp(name)) {
						let json = searchPath.getPackageJson(name);
						if (json.hash) { // is build
							this._local = searchPath.makePackage(name); // local pkg
							break;
						}
					}
				}
			} // if (local && local.build)
		}
	}

	private _fullPathname(relativePath: string) {
		return (this._pkgzFiles.has(relativePath) ? this._pkgzPath: this.path) + '/' + relativePath;
	}

	private _resolveAfter(key: string, pathname: string, hash: string) {
		let self = this, resolve = '';

		if (self._local) { // prioritize reading local package
			// if the file hash is same then using local path,
			// can avoid downloading new resources from the internet
			if ( self._local.filesHash[pathname] === hash ) {
				resolve = self._local._fullPathname(pathname);
			}
		}
		resolve = set_url_args(resolve || self._fullPathname(pathname), hash);
		let cached = {pathname, resolve};
		self._pathCache.set(key, cached);
		return cached;
	}

	/**
	 * @method resolve(relativePath:string)any
	 *
	 * Lookup internal module by the relative path
	*/
	resolve(relativePath: string): { pathname: string, resolve: string } {
		let self = this;
		self.installSync(); // install package

		if (self._localOnly) {
			return self._local!.resolve(relativePath);
		}

		let pathname = relativePath;
		let cached = self._pathCache.get(pathname);
		if (cached) {
			return cached;
		}
		if (!pathname) {
			pathname = String(self.json.main||'').replace(/^[\.\/\\]+/, ''); // clear invalid character
		}
		let extnames = Object.keys(Module._extensions);
		let pathnames = pathname ? [
			pathname,
			...extnames.map(e=>`${pathname}${e}`),
			...extnames.map(e=>`${pathname}/index'${e}`)
		]: extnames.map(e=>`index${e}`);

		// try using different extensions to search for and ` ${path name}/index`
		for (let path of pathnames) {
			let hash = self.filesHash[path];
			if (hash !== undefined) {
				return self._resolveAfter(relativePath, path, hash);
			}
		}
		if (!this.isHttp) { // try access the local file system
			for (let path of pathnames) {
				if ( isFileSync(self.path + '/' + path) ) { // find local
					return self._resolveAfter(relativePath, path, '');
				}
			}
		}

		throwModuleNotFound(self.path + '/' + pathname);
		throw '';
	}

	private _installComplete(prefix: string, cb?: Cb) {
		let self = this;
		if (self.build || isHttp(prefix)) {
			// read the version information of the package resource file.
			// if it is not in the build state, do not use cache
			let path = prefix + '/versions.json';
			let path_arg = set_url_args(path, self.hash/* || '__no_cache'*/);

			let ok = (text: string)=>{
				let {filesHash={},pkgzFiles={}} = parseJSON(text, path);
				let files = (self as {filesHash: Dict});
				self._status = PackageStatus.INSTALLED;
				if (self._pkgzPath) {
					for (let file of Object.keys(pkgzFiles)) {
						self._pkgzFiles.add(file);
					}
					files.filesHash = Object.assign(filesHash, pkgzFiles);
				} else {
					files.filesHash = Object.assign(pkgzFiles, filesHash);
				}
				cb && cb(); // ok
			};
			cb ? readText(path_arg).then(ok).catch(cb): ok(readTextSync(path_arg));
		} else {
			self._status = PackageStatus.INSTALLED;
			cb && cb(); // ok
		}
	}

	private _installFromHttp(cb?: Cb) {
		let self = this;
		if (self._local && self.hash == self._local.hash) { // two identical packages
			self._localOnly = true;
			self._status = PackageStatus.INSTALLED;
			return cb && cb();
		}

		if (!self.pkgzHash) { // No pkgz path
			return this._installComplete(self.path, cb);
		}

		// if the corresponding version of the file does not exist local then
		// downloading .pkgz file to local
		let pkgzHash = self.pkgzHash;
		let pathname = _fs.temp(`${self.name}.pkgz.${pkgzHash}`);
		// zip:///Users/pppp/sasa/aa.apk@/aaaaa/bbbb/aa.js

		if (_fs.existsSync(pathname)) { // file exists and not need downloaded
			// replace the path with the `zip:///` file reading protocol,
			// use the ` _fs.reader.readFile()` method to directly read the file
			self._pkgzPath = `zip:///${pathname.substring(8)}@`; // file:/// => zip:///
			self._installComplete(self._pkgzPath, cb);
		} else {
			// downloading ...
			let url = set_url_args(`${self.path}/${self.name}.pkgz`, pkgzHash);
			let save = pathname + '.~';
			// TODO:
			// when the file size is relatively large,
			// it is necessary to resume the download with a breakpoint,
			// and the download progress should be notified in real-time by reading the data stream
			let ok = function(err?: Error) { // download successful
				if (err) {
					if (self.tryLocal && self._local) { // try to query the local package
						print(err);
						self._localOnly = true;
						self._status = PackageStatus.INSTALLED;
						cb && cb();
					} else {
						throwErr(err, cb);
					}
				} else {
					_fs.renameSync(save, pathname);
					self._pkgzPath = `zip:///${pathname.substring(8)}@`; // file:/// => zip:///
					self._installComplete(self._pkgzPath, cb);
				}
			};

			if (cb) {
				_http.request({ url, save }, ok);
			} else {
				try {
					_http.requestSync({ url, save });
				} catch(err: any) {
					return ok(err);
				}
				ok();
			}
		} // if end
	}

	private _install(cb?: Cb) {
		let self = this;
		let path = self.path;

		if (self._status != PackageStatus.NO_INSTALL) {
			return throwErr(`${path} package installing repeat call`, cb);
		}
		self._status = PackageStatus.INSTALLING;

		if (!self.build) { // not build, debug state
			return self._installComplete(path, cb);
		}

		if (self.isHttp) { // is http
			return self._installFromHttp(cb);
		}

		// install local
		/*
		* build的pkg有两种格式
		* 1.package根目录存在.pkgz压缩文件,文件中包含全部文件版本信息与一部分源码文件以及资源文件.
		* 2.package根目录不存在.pkgz压缩文件,相比build前只多出文件版本信息,适用于android/ios安装包中存在.
		*/
		/* 文件读取器不能读取zip包内的.pkgz压缩文件.
		* 比如无法android资源包中的.pkgz文件
		* 所以android.apk中不能存在.pkgz格式文件否则将不能读取
		*/
		if (!isLocalZip(path) && // not a local zip protocol file
			isFileSync(`${path}/${self.name}.pkgz`)
		) { // there is a. pkgz file in the local package
			self._pkgzPath = `zip:///${path.substring(8)}/${self.name}.pkgz@`; // file:/// => zip:///xxx/xxx.pkgz@
			self._installComplete(self._pkgzPath, cb);
		}
		else { // there is no. pkgz package available locally
			self._installComplete(path, cb);
		}
	}

	/**
	 * Async install package
	*/
	async install(): Promise<void> {
		let self = this;
		if (self._status !== PackageStatus.INSTALLED) {
			if (self._local) { // install local pkg
				await self._local.install();
			}
			await new Promise<void>(function(resolve, reject) {
				self._install(function(err) {
					if (err) {
						self._status = PackageStatus.NO_INSTALL;
						reject(err);
					} else {
						resolve();
					}
				});
			});
		}
	}

	/**
	 * Sync install package
	*/
	installSync() {
		let self = this;
		if (self._status !== PackageStatus.INSTALLED) {
			if (self._local) { // install local
				self._local.installSync();
			}
			try {
				self._install();
			} catch(err) {
				self._status = PackageStatus.NO_INSTALL;
				throw err;
			}
		}
	}
}

/**
 * qk://quark ExtendModule Internal expansion module
 * 
 * @class QkPackage
 */
class QkPackage extends Package {
	constructor() {
		super('qk://quark', { name: 'quark', main: 'index.js', version: '1.0.0' });
		(['_common.js',
			'_event.js',
			'_ext.js',
			'_util.js',
			'action.js',
			'app.js',
			'buffer.js',
			'checkbox.js',
			'css.js',
			'ctr.js',
			'dialog.js',
			'errno.js',
			'event.js',
			'font.js',
			'fs.js',
			'http.js',
			'index.js',
			'keyboard.js',
			'media.js',
			'nav.js',
			'os.js',
			'bubbles.js',
			'package.json',
			'path.js',
			'pkg.js',
			'screen.js',
			'stepper.js',
			'storage.js',
			'test.js',
			'types.js',
			'util.js',
			'view.js',
			'window.js',
		]).forEach(e=>this.filesHash[e]='-');
		this._status = PackageStatus.INSTALLED;
	}

	resolve(relativePath: string): { pathname: string, resolve: string } {
		let self = this;
		let pathname = relativePath;
		let cached = self._pathCache.get(pathname);
		if (cached) {
			return cached;
		}
		if (!pathname) {
			pathname = self.json.main; // it must be a standard
		}
		let pathnames = [...Object.keys(Module._extensions), ''].map(e=>pathname+e);

		for (let pathname of pathnames) {
			if (this.filesHash.hasOwnProperty(pathname)) {
				cached = { pathname, resolve: this.path + '/' + pathname };
				self._pathCache.set(relativePath, cached);
				return cached;
			}
		}
		throwModuleNotFound(self.path + '/' + relativePath);
		throw '';
	}

	get isInternal() { return true } // yes internal package
}

new QkPackage();

/**
 * @default
*/
export default {

	/**
	 * @get mainPackage:Package
	*/
	get mainPackage() {
		return mainModule!.package;
	},

	/**
	 * @get mainModule:Module
	*/
	get mainModule() {
		return mainModule!;
	},

	/**
	 * @get mainFilename:string
	*/
	get mainFilename() {
		return mainModule!.filename;
	},

	/**
	 * Add global search path
	 * @param path global search directory path
	 * @param isFirst? use high priority
	 * @param noCache? use __no_cache param load pkgs.json file
	 */
	async addSearchPath(path: string, isFirst?: boolean, noCache?: boolean): Promise<void> {
		path = formatPath(path);
		if ( !searchPaths.hasOwnProperty(path) ) {
			await new SearchPath(path).load(noCache);
			isFirst ?
				globalPaths.unshift(path): globalPaths.push(path);
		}
	},

	/**
	 * Lookup package by package name
	*/
	lookupPackage(name: string, parent?: Module): Package | null {
		let r = lookup(name, parent);
		return r ? r.pkg: null;
	},
};