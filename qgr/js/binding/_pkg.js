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

'use strict';

const _util = requireNative('_util');
const _path = requireNative('_path');
const _fs = requireNative('_fs');
const _http = requireNative('_http');
const _pkgutil = requireNative('_pkgutil');
const { fallbackPath,
				resolvePathLevel,
				resolve, isAbsolute,
				isLocal, isLocalZip, 
				isNetwork, assert, debug, stripBOM } = _pkgutil;
const { readFile, 
				readFileSync, isFileSync,
				isDirectorySync, readdirSync } = requireNative('_reader');
const { haveNode } = _util;
const options = {};  // start options
const external_cache = {};
var ignore_local_package, ignore_all_local_package;
var keys = null, config = null;
var instance = null;
var Module, NativeModule;

function parse_keys(content) {
	if ( !keys ) {
		keys = requireNative('_keys');
	}
	return keys.parse(content);
}

function format_msg(args) {
	var msg = [];
	for (var i = 0; i < args.length; i++) {
		var arg = args[i];
		msg.push( arg instanceof Error ? arg.message: arg );
	}
	return msg;
}

function print_err(err) {
	console.error.apply(console, format_msg(arguments));
}

function print_warn(err) {
	console.warn.apply(console, format_msg(arguments));
}

function extendEntries(obj, extd) {
	for (var item of Object.entries(extd)) {
		obj[item[0]] = item[1];
	}
	return obj;
}

function __vx(raw_vx, attrs, vdata) {
	if (!raw_vx || raw_vx.vx !== 0) {
		throw new TypeError('Raw View xml type error.');
	}
	
	// { vx:0, v:[tag,attrs,childs,vdata] }

	var v = raw_vx.v.slice();
	var r = { vx: 0, v: v };
	
	if (vdata) {
		v[3] = vdata;
	}
	
	if (attrs.length != 0) {
		var raw_attrs = v[1];
		var attrs_map = {};
		v[1] = attrs; // new attrs
		for (var attr of attrs) {
			attrs_map[attr[0].join('.')] = 1; // mark current attr
		}
		for (var attr of raw_attrs) {
			var name = attr[0].join('.');
			if (!(name in attrs_map)) {
				attrs.push(attr);
				attrs_map[name] = 1;
			}
		}
	}

	return r;
}

/**
 * @fun err # create error object
 * @arg e {Object}
 * @ret {Error}
 */
function new_err(e) {
	if (! (e instanceof Error)) {
		if (typeof e == 'object') {
			e = extendEntries(new Error(e.message || 'Unknown error'), e);
		} else {
			e = new Error(e);
		}
	}
	return e;
}

/**
 * @fun cb # return default callback
 * @ret {Function}
 */
function new_cb(cb) {
	return cb || function () { };
}

/**
 * @fun throw # 抛出异常
 * @arg err {Object}
 * @arg [cb] {Function} # 异步回调
 */
function throw_err(e, cb) {
	new_cb(cb).throw(new_err(e));
}

// add url args
function set_url_args(path, arg) {
	if (/^(https?):\/\//i.test(path)) {
		var args = [];
		var url_arg = arg || options.url_arg;
		if ( url_arg ) {
			return path + (path.indexOf('?') == -1 ? '?' : '&') + url_arg;
		}
	}
	return path;
}

/**
 * @func read_text
 */
function read_text(path, cb) {
	return readFile(path, 'utf8', cb);
}

/**
 * @func read_text_sync
 */
function read_text_sync(path) {
	return readFileSync(path, 'utf8');
}

global.__vx = __vx;
global.__extend = extendEntries;

// -------------------------- Module extensions --------------------------

var Module_extensions = {};

// Native extension for .js
Module_extensions['.js'] = function(module, filename) {
	var content = read_text_sync(filename);
	var pkg = module.package;
	if (pkg && pkg.m_info.qgr_syntax) {
		if ( !pkg.m_build || 
			pkg.m_info.no_syntax_preprocess /*配置明确声明为没有进行过预转换*/ ) {
			content = _util.transformJs(content, filename);
		}
	}
	module._compile(stripBOM(content), filename);
};

// Native extension for .jsx
Module_extensions['.jsx'] = function(module, filename) {
	var content = read_text_sync(filename);
	var pkg = module.package;
	if ( !pkg || !pkg.m_build || 
		pkg.m_info.no_syntax_preprocess /*配置明确声明为没有进行过预转换*/ ) {
		content = _util.transformJsx(content, filename);
	}
	module._compile(stripBOM(content), filename);
};

// Native extension for .json
Module_extensions['.json'] = function(module, filename) {
	var content = read_text_sync(filename);
	module.exports = parseJSON(content, filename);
};

// Native extension for .keys
Module_extensions['.keys'] = function(module, filename) {
	var content = read_text_sync(filename);
	try {
		module.exports = parse_keys(content);
	} catch (err) {
		err.message = filename + ': ' + err.message;
		throw err;
	}
};

if (haveNode) {
	//Native extension for .node
	Module_extensions['.node'] = function(module, filename) {
	  filename = fallbackPath(filename);
	  return process.dlopen(module, path._makeLong(filename));
	};
}

// -------------------------- Package private API --------------------------

function parseJSON(source, filename) {
	try {
		return JSON.parse(stripBOM(source));
	} catch (err) {
		err.message = filename + ': ' + err.message;
		throw err;
	}
}

function Package_install3(self, path, cb) {
	// 读取package.json文件
	var info = self.m_info;
	info.src  = info.src || '';
	self.m_src  = resolve(self.m_path, info.src);
	if (info.name != self.m_name) {
		return throw_err('Lib name must be ' +
										 `consistent with the folder name, ${self.m_name} != ${info.name}`, cb);
	}
	
	if (self.m_build || isNetwork(path)) {
		// 读取package内部资源文件版本信息
		var versions_json = path + '/versions.json';
		var path2 = set_url_args(versions_json, self.m_version_code);

		var read_versions_ok = function(data) {
			data = parseJSON(data, versions_json);
			if (self.m_build) {
				self.m_pkg_files = data.pkg_files || {};// .pkg 中包含的文件列表 
			}
			self.m_versions = data.versions || {};
			self.m_install = true;
			cb && cb(); // ok
		}.catch(new_cb(cb).throw);

		cb ? read_text(path2, read_versions_ok) : read_versions_ok(read_text_sync(path2));
	} else {
		self.m_install = true;
		cb && cb(); // ok
	}
}

function Package_install_remote(self, cb) {
	// 如果本地不存在相应版本的文件,下载远程.pkg文件到本地.
	// 远程.pkg文件必须存在否则抛出异常,并且不使用备选2方式
	var version_code = self.m_version_code;
	var pathname = _path.temp(`${self.m_name}.pkg`);
	var pathname_ver = `${pathname}.${version_code}`;

	// zip:///Users/pppp/sasa/aa.apk@/aaaaa/bbbb/aa.js
	var has_pathname = _fs.existsSync(pathname);

	if (_fs.existsSync(pathname_ver)) { // 文件存在,无需下载
		// 设置一个本地zip文件读取协议路径,使用这种路径可直接读取zip内部文件
		if (has_pathname) {
			if (_fs.statSync(pathname).ino != _fs.statSync(pathname_ver).ino) { // 文件id不相同
				_fs.unlinkSync(pathname);
				_fs.linkSync(pathname_ver, pathname); // 链接
			}
		} else {
			_fs.linkSync(pathname_ver, pathname); // 链接
		}
		self.m_pkg_path = `zip:///${pathname_ver.substr(8)}@`;  // file:///
		Package_install3(self, self.m_pkg_path, cb);
	} else {
		var url = set_url_args(`${self.m_path}/${self.m_name}.pkg`, version_code);
		var tmp = pathname_ver + '.~';
		// TODO 文件比较大时需要断点续传下载
		// TODO 还应该使用读取数据流方式,实时回调通知下载进度
		let doanload_ok = function() { // 下载成功
			_fs.renameSync(tmp, pathname_ver);
			if (has_pathname)
				_fs.unlinkSync(pathname);
			_fs.linkSync(pathname_ver, pathname); // 链接
			self.m_pkg_path = `zip:///${pathname_ver.substr(8)}@`; // file:///
			Package_install3(self, self.m_pkg_path, cb);
		}.catch(function(err) {
			if (has_pathname) { // 使用原来的包
				console.error(err);
				self.m_pkg_path = `zip:///${pathname.substr(8)}@`;  // file:///
				Package_install3(self, self.m_pkg_path, cb);
			} else {
				new_cb(cb).throw(err);
			}
		});

		if (cb) {
			_http.download(url, tmp, doanload_ok);
		} else {
			try {
				_http.downloadSync(url, tmp);
			} catch(err) {  
				doanload_ok.throw(err); return;
			}
			doanload_ok();
		}
	}
}

function Package_install_local(self, receive, cb) {
	var path = receive.m_path;

	var install_ok = function() { // 保存旧文件版本信息
		receive.m_src = resolve(path, self.m_info.src);
		receive.m_versions = self.m_versions;
		receive.m_pkg_files = self.m_pkg_files;
		cb && cb();
	}.catch(new_cb(cb).throw);
	
	receive.m_pkg_path = '';

	/*
	* build的pkg有两种格式
	* 1.pkg根目录存在.pkg压缩文件,文件中包含全部文件版本信息与一部分源码文件以及资源文件.
	* 2.pkg根目录不存在.pkg压缩文件,相比build前只多出文件版本信息,适用于android/ios安装包中存在.
	*/
	/* 文件读取器不能读取zip内的.pkg文件. 
	* 比如无法android资源包中的.pkg文件 
	* 所以这里的pkg不能存在.pkg格式文件只能为普通文件
	*/

	if ( !isLocalZip(path) ) {
		var is_pkg = _fs.existsSync(`${path}/${self.m_name}.pkg`);
		if (is_pkg) {
			receive.m_pkg_path = `zip:///${path.substr(8)}/${self.m_name}.pkg@`;
		}
	}
	if (cb) {
		Package_install3(self, path, install_ok);
	} else {
		Package_install3(self, path)
		install_ok();
	}
}

// install
function Package_install2(self, cb) {
	if (self.m_build) { // pkg明确声明为已build状态
		if (self.m_local_launch) { // 本地发起pkg
			//
			var old = self.m_old;
			if (old) { // 先载入本地旧包,然后载入远程origin包
				let install_remote_ok = function(){ cb && cb() }.catch(err=>{
					// 不能安装远程包,
					console.error(err);
					extendEntries(self, old); // 恢复
					self.m_old = null;
					cb && cb();
				});
				if (cb) {
					// 读取旧pkg中的信息也许有些离散文件还可以继续使用,降低网络消耗
					Package_install_local(self, old, function() {
						Package_install_remote(self, install_remote_ok);
					}.catch(cb.throw));
				} else {
					Package_install_local(self, old);
					try {
						Package_install_remote(self);
					} catch(err) {
						install_remote_ok.throw(err);
					}
				}
			} else {

				if (self.m_origin) { 
					// 可能由于网络原因导致没有调用`Package_attempt_enable_origin()`启用源检测
					// 但本地可能存在原先下载的origin包,检测原先下载的远程包是否可用
					var path = _path.temp(`${self.m_name}.pkg`);
					if (_fs.existsSync(path)) { // 文件存在
						var pkg_path = `zip:///${path.substr(8)}@`;
						// 读取包内package.json文件内容
						var package_json = pkg_path + '/package.json';
						var info = parseJSON(read_text_sync(package_json), package_json);
						if (info.version_code != self.m_version_code && 
							info.build_time > self.m_info.build_time) { // 版本号不相同，并且时间比本地包新
							self.m_old = {
								m_path: self.m_path,
								m_version_code: self.m_version_code,
							};
							if (cb) {
								Package_install_local(self, self.m_old, function() {
									self.m_pkg_path = pkg_path;
									Package_install3(self, pkg_path, cb);
								}.catch(cb.throw));
							} else {
								Package_install_local(self, self.m_old);
								self.m_pkg_path = pkg_path;
								Package_install3(self, pkg_path);
							}
							return;
						}
					}
				}
				Package_install_local(self, self, cb);
			}
		} else { //  单纯的远程.pkg, 远程包一定都是.pkg
			Package_install_remote(self, cb);
		}
	} else {
		Package_install3(self, self.m_path, cb);
	}
}

// install
function Package_install(self, cb) {
	if ( self.m_install ) {
		return cb && cb();
	}
	if (cb) { // async
		Package_install2(self, function () {
			cb();
		}.catch(cb.throw));
	} else {
		Package_install2(self);
	}
}

function throw_MODULE_NOT_FOUND(request) {
	var err = new Error(`Cannot find module or file '${request}'`);
	err.code = 'MODULE_NOT_FOUND';
	throw err;
}

// 获取当前pkg内路径
function Package_get_path(self, pathname) {

	var rv = self.m_path_cache[pathname];
	if (rv) {
		return rv;
	}
	var ver;
	if (_path.extname(pathname)) {
		ver = self.m_versions[pathname];
		if ( ver === undefined ) { // 找不到版本信息
			if (isLocal(self.m_src)) {
				var src = self.m_src + '/' + pathname;
				if (isFileSync(src)) { // 尝试访问文件系统,是否能找到文件信息
					ver = '';
				}
			}
			if (ver !== '') {
				throw_MODULE_NOT_FOUND(pathname);
			}
		}
	} else { // 没有扩展名,尝试使用多个扩展名查找 .js .jsx .json .keys
		var extnames = Object.keys(Module._extensions);
		// 尝试使用尝试默认扩展名不同的扩展名查找
		for (var ext of extnames) {
			ver = self.m_versions[pathname + ext];
			if (ver !== undefined) {
				pathname += ext;
				break;
			}
		}
		if ( ver === undefined ) {
			if (isLocal(self.m_src)) { // 尝试访问本地文件系统,是否能找到文件信息
				for (var ext of extnames) {
					var src = self.m_src + '/' + pathname + ext;
					if ( isFileSync(src) ) {
						pathname += ext;
						ver = ''; break;
					}
				}
			}
			if (ver === undefined) {
				throw_MODULE_NOT_FOUND(pathname);
			}
		}
	}

	rv = self.m_src + '/' + pathname;
	if ( self.m_pkg_files[pathname] ) { // 使用.pkg
		rv = self.m_pkg_path + '/' + pathname;
	}
	if (self.m_old) { // 读取本地旧文件
		var old = self.m_old;
		// 版本相同,完全可以使用本地旧文件路径,这样可以避免从网络下载新资源
		if ( old.m_versions[pathname] === ver ) {
			if ( old.m_pkg_files[pathname] ) {
				rv = old.m_pkg_path + '/' + pathname;
			} else {
				rv = old.m_src + '/' + pathname;
			}
		}
	}

	self.m_path_cache[pathname] = rv = set_url_args(rv, ver);
	return rv;
}

function resolve_filename(request) {
	if (haveNode && NativeModule.nonInternalExists(request)) {
		return request;
	}
	var mat = request.match(/^([a-z\-_$]+)(\/(.+))?$/i);
	if (mat) {
		var pkg = instance.getPackage(mat[1]);
		if (pkg) {
			Package_install(pkg);
			return Package_get_path(pkg, mat[3] ? resolvePathLevel(mat[3]) : '');
		}
	} else if (isAbsolute(request)) {
		var pathname = resolve(request);
		var o = instance.getPackageWithAbsolutePath(pathname);
		if (o) {
			return Package_get_path(o.package, o.path);
		} else {
			if (pathname in external_cache || isFileSync(pathname)) {
				return pathname;
			}
		}
	}
	throw_MODULE_NOT_FOUND(request);
}

function Package_resolve_filename(self, dir, request) {
	if (haveNode && NativeModule.nonInternalExists(request)) {
		return request;
	}
	if (request.length > 2 && 
			request.charCodeAt(0) === 46/*.*/ &&  (
			request.charCodeAt(1) === 46/*.*/ || 
			request.charCodeAt(1) === 47/*/*/)
	) {
		// path in pkg
		return Package_get_path(self, resolvePathLevel(dir + '/' + request));
	}
	return resolve_filename(request);
}

/** 
 * 如果没有path, require main file 
 */
function Package_require(self, parent, request) {
	Package_install(self);

	if (!request || request == '.') {
		request = self.m_info.main || 'index';
	}
	request = resolvePathLevel(request, true);
	var pathname = Package_get_path(self, request);
	var module = self.m_modules[pathname];
	if (module) return module;

	module = new Module(self.m_src + '/' + request, parent, self);
	module.file = request;
	module.dir = _path.dirname(request);
	self.m_modules[pathname] = module;

	var name = _path.basename(request);
	name = name.substr(0, name.length - _path.extname(name).length).replace(/[\.\-]/g, '_');
	var exports = module.exports[name] = module.exports;

	var threw = true;
	try {
		module.load(pathname);
		threw = false;
	} finally {
		if (threw) {
			delete self.m_modules[pathname];
		}
	}

	exports = module.exports;

	if (!(name in exports)) {
		exports[name] = exports;
	}

	return module;
}

/**
 * @func Package_attempt_enable_origin 必需要install之前调用,安装后调用无效
 */
function Package_attempt_enable_origin(self, origin, origin_version_code, origin_info) {
	if (Package_is_can_check_origin(self, origin)) {
		if ( origin_version_code != self.m_version_code ) {
			self.m_old = {
				m_path: self.m_path, 
				m_version_code: self.m_version_code,
				m_info: self.m_info,
			};
			self.m_path = origin;
			self.m_version_code = origin_version_code;
			self.m_info = origin_info;
		}
	}
}

/**
 * @func Package_is_can_check_origin 是否可以检查源
 */
function Package_is_can_check_origin(self, origin) {
	if ( self.m_build && !self.m_install && self.m_local_launch ) {
		if (origin == self.m_origin && origin != self.m_path) {
			return true;
		}
	}
	return false;
}

/**
 * @class Package
 */
class Package {

	get info() { return this.m_info }
	get origin() { return this.m_origin }
	get name() { return this.m_name }   // pkg 名称
	get path() { return this.m_path }   // pkg 路径
	get src() { return this.m_src }
	get has_build() { return this.m_build }             // build
	get versions() { return this.m_versions }           // 资源版本
	get version_code() { return this.m_version_code }   // 版本代码
	
	/**
	 * @constructor
	 */
	constructor(path, name, is_build, info, version_code, origin) {
		var self = this;
		self.m_info = info;
		self.m_origin = isNetwork(origin) ? origin : '';
		self.m_local_launch = isLocal(path);
		self.m_name = name;
		self.m_path = path;
		self.m_src = '';        /* */
		self.m_build = is_build;
		self.m_pkg_path = '';   /* .pkg 文件本地路径 zip:///temp/test.pkg@ */
		self.m_pkg_files = {};  /* .pkg 中的文件列表,没有.pkg文件这个属性为null */
		self.m_versions = {};   /* .pkg 包内文件的版本信息 */
		self.m_version_code = version_code;
		self.m_install = false;
		self.m_modules = {};
		self.m_old = null;
		self.m_path_cache = {};
		//zip:///applications/test.apk@/

		var dirname = path.match(/([^\/]+)$/)[1];
		if (dirname != name) {
			throw new Error(`Lib name must be consistent with the folder name, ${dirname} != ${name}`);
		}
	}
	
	/**
	 * 获取路径
	 */
	resolve(path) {
		Package_install(this);
		return Package_resolve_filename(this, '', path);
	}
}

// -------------------------- Packages private func --------------------------

// 注册更多相同名称的pkg都没关系,最终都只使用最开始注册的pkg
function Packages_register_path(self, path, optional) {
	var path2 = resolve(path);
	var register = self.m_pkgs_register[path2];
	if ( !register ) {
		var mat = path2.match(/^(.+?\/)(?:([^\/]+)\/)?([a-z_$][a-z0-9\-_$]*)$/i);
		if ( mat ) {
			if ( mat[2] ) { // add libs
				Packages_add_pkg_search_path(self, mat[1] + mat[2]);
			}
			/* pkg的状态,-1忽略/0未就绪/1准备中/2已经就绪/3异常 */
			self.m_pkgs_register[path2] = register = {
				ready: 0, 
				path: path2, 
				name: mat[3], 
				optional: !!optional,
			};
			self.m_is_ready = false; /* 设置未准备状态 */
		} else {
			throw new Error(`Invalid pkg path "${path}"`);
		}
	}
	return register;
}

function Packages_unregister_path(self, path) {
	path = resolve(path);
	var register = self.m_pkgs_register[path];
	if (register) {
		register.ready = -1;
		delete self.m_pkgs_register[path];
	}
}

function Packages_add_pkg_search_path(self, libs) {
	if (libs) {
		var path = resolve(libs);
		if ( !self.m_search_path[path] ) {
			/* `pkg search path`的状态,0未就绪/1准备中/2已经就绪 */
			self.m_search_path[path] = { ready: 0, path: path };
			self.m_is_ready = false; /* 设置未准备状态 */
		}
	}
}

function assert_origin(self, path) {
	var register = self.m_pkgs_register[resolve(path)];
	if ( !register ) {
		throw new Error('unregister "' + path + '"');
	}
	if (register.ready != 0 || register.ready != 1 ) { 
		throw new Error('It cannot be modified now origin "' + path + '"');
	}
	return register;
}

function Packages_set_origin(self, path, origin) {
	assert_origin(self, path).origin = origin || '';
}

function Packages_disable_origin(self, path, disable) {
	assert_origin(self, path).disable_origin = !!disable;
}

function Packages_depe_pkg(self, path, depe) {
	function add_depe(pathname) {
		if ( isAbsolute(i) ) {
			Packages_register_path(self, pathname);
		} else {
			Packages_register_path(self, path + '/' + pathname);
		}
	}
	if ( typeof depe == 'object' ) {
		if ( Array.isArray(depe)) {
			depe.forEach(add_depe);
		} else {
			for ( var i in depe ) { // 添加依赖路径
				add_depe(i);
			}
		}
	}
}

function Packages_new_pkg(self, path, name, is_build, info, version_code, origin) {
	if (typeof info.build_time != 'number') {
		info.build_time = 0;
	}
	info.src = info.src || '';
	var pkg = self.m_pkgs[name];
	if (pkg) { // Pcakage 实例已创建(尝试更新)
		if (is_build)
			Package_attempt_enable_origin(pkg, path, version_code, info);
	} else {
		pkg = new Package(path, name, is_build, info, version_code, String(origin)); // 创建一个pkg
		self.m_pkgs[name] = pkg;
		
		if ( pkg.m_origin ) { // reg origin pkg and check update
			Packages_register_path(self, pkg.m_origin, true);
		}
	}
}

//
// 通过一个在pkg父亲目录下pkgs.key的描述列表文件创建pkg对像
// 如果这些pkg在一个http服务器，通过这个packages.json文件能避免下载所有的package.json
// 因为packages.json包含这个目录中所有pkg的简单描述,这个文件一般会由开发工具创建
//
function Packages_parse_new_pkgs_json(self, node_path, content, local) {
	if ( node_path.ready == 2 ) return;
	node_path.ready = 2;
	node_path = node_path.path;
	
	var packages = JSON.parse(content);
	
	for ( var name in packages ) {
		var info = packages[name]; 
		if (name[0] != '@' /* 忽略: @ */ && typeof info == 'object' &&
			(!local || ignore_local_package.indexOf(name) === -1))
		{
			var path  = node_path + '/' + name; // pkg path
			if (info.path) {
				path = resolve(isAbsolute(info.path) ? 
					info.path : node_path + '/' + info.path);
			}
			var version_code = String(info.version_code || ''); // pkg version code
			// is pkg build, 是否为build过的代码
			// 指定一个最终build的版本代码也可视目标pkg为build过后的代码
			var is_build = '_build' in info ? !!info._build : !!version_code;
			var origin = info.origin || '';
			
			var register = self.m_pkgs_register[path];
			if ( register ) {
				origin = register.disable_origin ? '' : register.origin || origin;
			} else {
				self.m_pkgs_register[path] = register = { ready: 0, path: path, name: name };
			}

			if ( Packages_verification_is_need_load_pkg(self, register, false) ) {
				register.ready = 2;
				Packages_depe_pkg(self, path, info.external_deps);
				Packages_new_pkg(self, path, name, is_build, info, version_code, origin);
			}
		}
	}
}

function Packages_verification_is_need_load_pkg(self, register, is_warn) {
	var pkg = self.m_pkgs[register.name];
	if ( pkg ) { // pkg 已创建
		if ( pkg.path == register.path )  // 路径相同不需要在做任何工作了
			return false;
		// 路径不相同检测是否可以更新替换
		if ( !Package_is_can_check_origin(pkg, register.path) ) { // 不需要check
			register.ready = -1; // 忽略
			if ( is_warn ) {
				console.warn('Ignore, Lib has been created and cannot be replaced,', register.path);
			}
			return false;
		}
	}
	return true;
}

function Packages_parse_new_pkg_json(self, register, content) {
	var info = parseJSON(content, register.path + '/package.json');
	var version_code = String(info.version_code || '');
	var is_build = '_build' in info ? !!info._build : !!version_code;
	var origin = register.disable_origin ? '' : register.origin || info.origin || '';
	
	register.ready = 2; // 完成
	
	Packages_depe_pkg(self, register.path, info.external_deps);
	Packages_new_pkg(self, register.path, 
		info.name, is_build, info, version_code, origin);
}

function Packages_load_pkg_json(self, register, async, receipt) {

	if ( Packages_verification_is_need_load_pkg(self, register, true) ) {
		// 没有此pkg实例,尝试读取package.json文件
		// 文件必须强制加载不使用缓存
		var package_json = register.path + '/package.json';
		var pkg_json = set_url_args(package_json, '__nocache');
		var err = null;
		if ( async ) {
			read_text(pkg_json, function(content) {
				try {
					Packages_parse_new_pkg_json(self, register, content);
				} catch (e) { 
					err = e;
				}
				Packages_load_pkg_json_after(self, err, register, true, receipt);
			}.catch(function(err) {
				Packages_load_pkg_json_after(self, err, register, true, receipt);
			}));
			return;
		} else {
			try {
				Packages_parse_new_pkg_json(self, register, read_text_sync(pkg_json));
			} catch(e) {
				err = e;
			}
		}
	}
	Packages_load_pkg_json_after(self, err, register, async, receipt);
}

function Packages_load_pkg_json_after(self, err, register, async, receipt) {
	if (err) {
		if (register.optional) { // optional
			register.ready = -1; // ignore
			if (receipt)
				Packages_require_before(self, async);
		} else {
			var async_cb = self.m_async_cb;
			register.ready = 3; // 设置为异常
			register.error = err;
			self.m_async_cb = [];
			async_cb.forEach(function(cb) { 
				cb.throw(err); /* 抛出异常 */ 
			});
			if (!async) {
				throw err;
			}
		}
	} else {
		if (receipt)
			Packages_require_before(self, async);
	}
}

function Packages_try_parse_new_pkgs_json(self, node_path, async, local) {
	var pkgs_json = node_path.path + '/packages.json';
	// load packages.json `packages.json` 文件必须强制加载不使用缓存
	var json_path_no_cache = set_url_args(pkgs_json, '__nocache');
	if (async) {
		node_path.ready = 1;  // 载入中packages.json
		read_text(json_path_no_cache, function(content) {
			try {
				Packages_parse_new_pkgs_json(self, node_path, content, local);
			} catch (err) { 
				node_path.ready = 2;
				print_warn(err, `Ignore load ${pkgs_json}`);
			}
			Packages_require_before(self, true);
		}.catch(function(err) {
			node_path.ready = 2;
			print_warn(err, `Ignore load ${node_path.path}`);
			Packages_require_before(self, true);
		}));
	} else {
		try {
			var content = read_text_sync(json_path_no_cache);
			Packages_parse_new_pkgs_json(self, node_path, content, local);
		} catch (err) {
			node_path.ready = 2;
			print_warn(err, `Ignore load ${node_path.path}/packages.json`);
		}
	}
}

// 准备工作，实例化已注册过的所有pkg,并读取packages.json描述文件
// require befory ready
function Packages_require_before(self, async, cb) {
	if (self.m_is_ready) {
		return cb && cb();
	}
	if (self.m_async_cb.length && !async) { // 正在异步载入中,完成前禁止再使用同步
		throw new Error('Now can not be loaded synchronously because an ' +
										'asynchronous loading operation is being carried out.');
	}
	if (async && cb) {
		self.m_async_cb.push(cb);
	}
	var is_loading = false;

	// Prioritizing local loading pkg search path
	for ( var i in self.m_search_path ) {
		var node_path = self.m_search_path[i];
		if (node_path.ready === 0) {
			if (isLocal(node_path.path)) { // local
				if (!ignore_all_local_package && isDirectorySync(node_path.path)) {
					//  Give priority to the use of `packages.json`
					if (isFileSync(node_path.path + '/packages.json')) { 
						Packages_try_parse_new_pkgs_json(self, node_path, false, true);
					} else { // no packages.json
						readdirSync(node_path.path).forEach(function(dirent) {
							if (dirent.type === 2 && ignore_local_package.indexOf(dirent.name) == -1) {
								if ( isFileSync(dirent.pathname + '/package.json') ) {
									var register = Packages_register_path(self, dirent.pathname);
									if (register.ready === 0)
										Packages_load_pkg_json(self, register, false, false);
								}
							}
						});
					}
				} else {
					node_path.ready = 2;  // ok
				}
			}
		}
	}

	// Loading network pkg search path
	for ( var i in self.m_search_path ) {
		var node_path = self.m_search_path[i];
		if (node_path.ready === 0) {
			if (!isLocal(node_path.path)) { // local
				if (async) {       // network
					is_loading = true;      // 1.载入中,2.完成
					Packages_try_parse_new_pkgs_json(self, node_path, true);
				} else { // sync network
					Packages_try_parse_new_pkgs_json(self, node_path, false);
				}
			}
		} else if (node_path.ready === 1) {
			if (async) {
				is_loading = true; // 1.载入中,2.完成
			}
		}
	}

	if ( is_loading ) return;
	
	// Loading packages
	for ( var i in self.m_pkgs_register ) {
		var register = self.m_pkgs_register[i];
		if (register.ready === 0) {
			is_loading = true;
			register.ready = 1; // 设置成加载中状态
			Packages_load_pkg_json(self, register, async, true);
		} else if (register.ready === 1) {
			is_loading = true;
		} else if ( register.ready == 3 ) { // err
			throw_err(`Load pkg fail "${register.path}"\n${register.error.message}`, cb);
			return;
		}
	}
	
	if ( !is_loading ) { // 没有任何载入中
		var async_cb = self.m_async_cb;
		self.m_is_ready = true;
		self.m_async_cb = [];
		async_cb.forEach(function(cb) {
			cb();
		});
	}
}

/**
 * @class Packages
 */
class Packages {

	constructor() {
		assert(!instance);
		instance = this;
		this.m_search_path = {};
		this.m_pkgs_register = {};   // all register path
		this.m_pkgs = {};            // 当前加载的pkgs
		this.m_async_cb = [];        //
		this.m_is_ready = true;      // 是否已准备就绪
		this.m_main_module = null;
	}

	/**
	 * @get mainModule
	 */
	get mainModule() {
		return this.m_main_module;
	}

	/**
	 * @get mainPackage
	 */
	get mainPackage() {
		return this.m_main_module && this.m_main_module.package;
	}
	
	/**
	 * @get main main run file path
	 */
	get mainFilename() {
		return this.m_main_module && this.m_main_module.filename;
	}
	
	/**
	 * @get pkgs 获取pkgs 名称列表
	 */
	get names() {
		var rev = [];
		for (var i in this.m_pkgs) {
			rev.push(i);
		}
		return rev;
	}
	
	/**
	 * 是否有这个pkg
	 */
	hasPackage(name) {
		return name in this.m_pkgs;
	}
	
	/**
	 * 通过名称获取pkg实体
	 */
	getPackage(name) {
		return this.m_pkgs[name];
	}
	
	/**
	 * @fun getPackageWithAbsolutePath 通过绝对路径获取pkg内部相对路径,没有找到返回null
	 * @arg path {String} 绝对路径
	 */
	getPackageWithAbsolutePath(path) {
		for (var i in this.m_pkgs) {
			var pkg = this.m_pkgs[i];
			var old = pkg.m_old;
			
			if (path.indexOf(pkg.path) === 0 || 
					(old && path.indexOf(old.m_path) === 0) ) { // 可能匹配
				Package_install(pkg);
				
				var src;
				
				if (path.indexOf(pkg.src) === 0) {
					src = pkg.src;
				} else if (old && path.indexOf(old.m_src) === 0) {
					src = old.m_src;
				}
				
				if (src) {
					return {
						package: pkg, 
						path: path.substr(src.length + 1) || pkg.m_info.main || 'index',
					};
				}
			}
		}
		return null;
	}

	/**
	 * @func addPackageSearchPath(libs) 
	 */
	addPackageSearchPath(libs) {
		Packages_add_pkg_search_path(this, libs);
	}
	
	/**
	 * @func addPackage
	 */
	addPackage(packagePath) {
		Packages_register_path(this, packagePath);
	}

	/**
	 * @func setOrigin(path,origin)
	 */
	setOrigin(packagePath, origin) {
		Packages_set_origin(this, path, origin);
	}

	/**
	 * @func disableOrigin(path)
	 */
	disableOrigin(packagePath, disable) {
		Packages_disable_origin(this, path, disable);
	}

	/**
	 * @func load Asynchronous mode load packages info and ready
	 */
	load(packageNames, cb) {
		assert(typeof cb == 'function', 'Callbacks must be used');
		if ( !Array.isArray(packageNames) ) {
			packageNames = [ packageNames ];
		}

		var len = packageNames.length;
		var count = 0;
		var e = null;
		var self = this;
		
		var callback = function() {
			if ( !e) {
				cb((++count) / len); // err, process
			}
		}.catch(function(err) {
			if ( !e) {
				e = err; cb.throw(e);
			}
		});
		
		Packages_require_before(this, true, function() {
			for (var i = 0; i < len; i++) {
				var pkg = self.getPackage(packageNames[i]);
				if ( ! pkg) {
					return throw_err(`Lib "${packageNames[i]}" does not exist`, cb);
				}
				Package_install(pkg, callback);
			}
		}.catch(cb.throw));
	}

	get options() {
		return options;
	}

	/**
	 * @get config
	 */ 
	get config() {
		if (!config) {
			config = {};
			var pkg = this.mainPackage;
			if (pkg) {
				config = 
					inl_require_without_err(pkg.name + '/config') ||
					inl_require_without_err(pkg.name + '/.config') ||
					inl_require_without_err(_path.cwd() + '/config') || 
					inl_require_without_err(_path.cwd() + '/.config') || {};
			} else {
				config = 
					inl_require_without_err(_path.cwd() + '/config') || 
					inl_require_without_err(_path.cwd() + '/.config') || {};
			}
		}
		return config;
	}

	_resolveFilename(request, parent) {
		if (parent) {
			return Package_resolve_filename(parent.package, parent.dir, request);
		} else {
			return resolve_filename(request);
		}
	}

	/**
	 * # startup run
	 * # parse args ready run
	 */
	_startup(_Module, _NativeModule) {
		assert(_Module);
		exports.Module = Module = _Module;
		exports.NativeModule = NativeModule = _NativeModule || null;
		Object.assign(Module._extensions, Module_extensions);

		parse_argv(); // parse argv

		var main = _util.argv[1];
		if (main) { // start
			add_main_search_path(main);
			
			if ( /^.+?\.jsx?$/i.test(main) ) { // js or jsx
				inl_require(main, null);
			} else {
				var mat = main.match( /^(.+\/)?([a-z_$][a-z0-9_\-$]*)$/i );
				if ( !mat) { // pkg
					throw new Error(`Could not start, invalid package path "${main}"`);
				}
				var pkg = mat[2];
				instance.addPackage(main); // 添加pkg
				
				if (options.dev) { // sync 这样更加容易发现错误,但会卡死工作线程
					inl_require(pkg, null); // 载入pkg
				} else {
					instance.load(pkg, function(proc) { // 异步载入`package`
						if (proc == 1) 
							inl_require(pkg, null);
					}.catch(function(err) {
						throw err;
					}));
				}
			}
		}
		delete Packages.prototype._startup;
	}

	_require(request, parent) {
		return inl_require(request, parent);
	}

}

/**
 * @func extend(obj, extd)
 */
Packages.prototype.extendEntries = extendEntries;
Packages.prototype.resolve = resolve;
Packages.prototype.isAbsolute = isAbsolute;
Packages.prototype.isLocal = isLocal;
Packages.prototype.isLocalZip = isLocalZip;
Packages.prototype.isNetwork = isNetwork;

// require absolute path file
function inl_require_external(path) {
	var r = instance.getPackageWithAbsolutePath(path);
	if (r) { // 重新require
		return inl_require(r.package, '', r.path);
	}
	// 文件不在pkg内部,这是一个外部文件
	// 是否载入过这个文件
	if (external_cache[path]) {
		return external_cache[path].exports;
	}

	var module = new Module(path, null, null);
	external_cache[path] = module;

	var threw = true;
	try {
		module.load(set_url_args(path));
		threw = false;
	} finally {
		if (threw) {
			delete external_cache[path];
		}
	}
	return module.exports;
}

/**
 * require('qgr/util.js');
 * require('qgr/gui');
 *
 * @fun inl_require
 * @arg request {String}  #    请求名
 * @arg parent {Module}   #    父模块
 */
function inl_require(request, parent) {
	var pkg = null;
	var dir = '';

	if (parent) {
		debug('Module._load REQUEST %s parent: %s', request, parent.id);
		pkg = parent.package;
		dir = parent.dir;
	}

	Packages_require_before(instance, false); //先准备pkg

	if (haveNode && NativeModule.nonInternalExists(request)) {
		debug('load native module %s', request);
		return NativeModule.require(request);
	}

	if (request.length > 2 && 
			request.charCodeAt(0) === 46/*.*/ &&  (
			request.charCodeAt(1) === 46/*.*/ || 
			request.charCodeAt(1) === 47/*/*/)
	) {
		// path in package
	} else if (isAbsolute(request)) { // absolute path
		return inl_require_external(resolve(request));
	} else {
		var mat = request.match(/^([a-z\-_\$]+)(\/(.+))?$/i);
		if (mat) { // 导入一个新的package的名称,如果pkg不存在会抛出异常
			pkg = instance.getPackage(mat[1]);
			request = mat[3] || '';
			dir = '';
			if (!pkg) { // 这是错误的, require('test/xx'); 这个 test package 必须存在
				throw throw_err(`require error, "${mat[1]}" pkg not register`);
			}
		}
	}
	
	request = dir ? dir + '/' + request : request; // 包内路径

	if (pkg) {
		return Package_require(pkg, parent, request).exports;
	} else { // 外部导入 `inl_require_external`
		return inl_require_external(resolve(request));
	}
}

// without error
function inl_require_without_err(pathname, parent) {
	try {
		return inl_require(pathname, parent);
	} catch(e) {}
}

function parse_argv() {
	var args = _util.argv.slice(2);
	
	for (var i = 0; i < args.length; i++) {
		var item = args[i];
		var mat = item.match(/^-{1,2}([^=]+)(?:=(.*))?$/);
		if (mat) {
			var name = mat[1].replace(/-/gm, '_');
			var val = mat[2] || 1;
			var raw_val = options[name];
			if ( raw_val ) {
				if ( Array.isArray(raw_val) ) {
					raw_val.push(val);
				} else {
					options[name] = [raw_val, val];
				}
			} else {
				options[name] = val;
			}
		}
	}

	if (haveNode) {
		if (process.execArgv.some(s=>(s+'').indexOf('--inspect') == 0)) {
			options.dev = 1;
		}
	}

	options.dev = !!options.dev;
	
	if ( !('url_arg' in options) ) {
		options.url_arg = '';
	}

	if ('no_cache' in options || options.dev) {
		if (options.url_arg) {
			options.url_arg += '&__nocache';
		} else {
			options.url_arg = '__nocache';
		}
	}

	ignore_local_package = options.ignore_local || [];
	ignore_all_local_package = false;
	if ( typeof ignore_local_package == 'string' ) {
		if ( ignore_local_package == '' || ignore_local_package == '*' ) {
			ignore_all_local_package = true;
		} else {
			ignore_local_package = [ ignore_local_package ];
		}
	} else {
		ignore_all_local_package = ignore_local_package.indexOf('*') != -1;
	}
}

function add_main_search_path(main) {

	if (isNetwork(main)) {
		if (options.dev) {
			Packages_require_before(instance); // load packages
			// 这是一个网络启动并为调式状态时,尝试从调式服务器`/libs` load `package`
			var mat = main.match(/^https?:\/\/[^\/]+/);
			assert(mat, 'Unknown err');
			instance.addPackageSearchPath(mat[0] + '/libs');
		}
	}
	else { // local
		if (_path.extname(main) == '') { // package
			instance.addPackageSearchPath(main + '/libs');
			instance.addPackageSearchPath(main + '/../libs');
		} else {
			instance.addPackageSearchPath(_path.dirname(main) + '/libs');
			instance.addPackageSearchPath(_path.dirname(main) + '/../libs');
		}
	}

	[
		_path.resources(), 
		_path.resources('libs'),
	].concat(Module.globalPaths).forEach(function(path) {
		instance.addPackageSearchPath(path);
	});
}

exports.packages = new Packages();