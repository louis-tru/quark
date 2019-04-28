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

var util = require('qkit');
var fs = require('qkit/fs');
var child_process = require('child_process');
var keys = require('qkit/keys');
var path = require('qkit/path');
var Buffer = require('buffer').Buffer;
var paths = require('./paths');
var uglify = require('./uglify');
var { syscall } = require('qkit/syscall');

var base64_chars =
	'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-'.split('');
	
function exec_cmd(cmd) {
	var r = child_process.spawnSync('sh', ['-c', cmd]);
	if (r.status != 0) {
		if (r.stdout.length) {
			console.log(r.stdout.toString('utf8'));
		}
		if (r.stderr.length) {
			console.error(r.stderr.toString('utf8'));
		}
		process.exit(0);
	} else {
		var rv = [];
		if (r.stdout.length) {
			rv.push(r.stdout.toString('utf8'));
		}
		if (r.stderr.length) {
			rv.push(r.stderr.toString('utf8'));
		}
		return rv.join('\n');
	}
}

function new_zip(self, cwd, source, target) {
	console.log('Out ', path.basename(target));
	exec_cmd('cd ' + cwd + '; rm -r ' + target + '; zip ' + target + ' ' + source.join(' '));
}

function unzip(self, source, target) {
	exec_cmd('cd ' + target + '; unzip ' + source);
}

function jsa_shell(source, target) {
	var os = process.platform == 'darwin' ? 'osx': process.platform;
	var jsa_shell = `${__dirname}/bin/${os}-jsa-shell`;
	if ( fs.existsSync(jsa_shell) ) {
		exec_cmd(`${jsa_shell} ${source} ${target} --clean-comment`);
	} else {
		throw new Error(`Cannot find jsa-shell command`);
	}
}

function parse_json_file(filename) {
	try {
		return JSON.parse(fs.readFileSync(filename, 'utf-8'));
	} catch (err) {
		err.message = filename + ': ' + err.message;
		throw err;
	}
}

var Hash = util.class('Hash', {
	
	m_hash: 5381,
	
	update_str: function (input) {
		var hash = this.m_hash;
		for (var i = input.length - 1; i > -1; i--) {
			hash += (hash << 5) + input.charCodeAt(i);
		}
		this.m_hash = hash;
	},
	
	update_buff: function (input) {
		var hash = this.m_hash;
		for (var i = input.length - 1; i > -1; i--) {
			hash += (hash << 5) + input[i];
		}
		this.m_hash = hash;
	},
	
	update_buff_with_len: function (input, len) {
		var hash = this.m_hash;
		for (var i = len - 1; i > -1; i--) {
			hash += (hash << 5) + input[i];
		}
		this.m_hash = hash;
	},
	
	digest: function () {
		var value = this.m_hash & 0x7FFFFFFF;
		var retValue = '';
		do {
			retValue += base64_chars[value & 0x3F];
		}
		while ( value >>= 6 );
		return retValue;
	},
});

function console_log(self, tag, pathname) {
	console.log(tag, self.m_cur_pkg_name + '/' + pathname);
}

// 获取跳过文件列表
// "name" pkg 名称
function get_skip_files(self, pkg_json, name) {
	var rev = [ ];
	
	if (pkg_json.skip) {
		if (Array.isArray(pkg_json.skip)) {
			rev = pkg_json.skip;
		} else {
			rev = [ String(pkg_json.skip) ];
		}
		delete pkg_json.skip;
	}
	
	if ( !pkg_json.src ) {
		rev.push('native');
	}
	rev.push('node_modules');
	rev.push('libs');
	rev.push('package.json');
	rev.push('versions.json');
	
	var reg = new RegExp('^:?' + name + '$');
	self.skip.forEach(function (src) {
		var ls = src.split('/');
		if (reg.test(ls.shift()) && ls.length) {
			rev.push(ls.join('/'));
		}
	});
	
	return rev;
}

// 获取分离文件列表
function get_detach_files(self, pkg_json, name) {
	var rev = [];
	
	if (pkg_json.detach) {
		if (Array.isArray(pkg_json.detach)) {
			rev = pkg_json.detach;
		} else {
			rev = [ String(pkg_json.detach) ];
		}
		delete pkg_json.detach;
	}
	
	var reg = new RegExp('^:?' + name + '$');
	self.detach.forEach(function (src) {
		var ls = src.split('/');
		if (reg.test(ls.shift()) && ls.length) {
			rev.push(ls.join('/'));
		}
	});
	return rev;
}

function build_pkg(self, pathname, ignore_depe) {
	return build_pkg1(self, pathname, 
		self.m_target_local, 
		self.m_target_public, 0, ignore_depe);
}

// build pkg item
function build_pkg1(self, pathname, target_local, target_public, ignore_public, ignore_depe) {
	var source_path = path.resolveLocal(pathname);
	var name = path.basename(source_path);
	var target_local_path = target_local + '/' + name;
	var target_public_path = target_public + '/' + name;
	
	// ignore network pkg 
	if ( /^https?:\/\//i.test(source_path) ) { 
		return { absolute_path: source_path, relative_path: absolute_path };
	}

	var out = self.m_output_pkgs[name];
	if ( out ) { // Already complete
		return out;
	}
	
	var pkg_json = parse_json_file(source_path + '/package.json');
	
	util.assert(pkg_json.name && pkg_json.name == name, 
							'Lib name must be consistent with the folder name, ' + 
							name  + ' != ' + pkg_json.name);

	self.m_output_pkgs[name] = out = { pkg_json: pkg_json };

	var skip_install = pkg_json.skipInstall && pkg_json.origin;

	if ( skip_install ) {
		out.absolute_path = pkg_json.origin;
	} else {
		out.absolute_path = '../' + name;
	}
	out.relative_path = '../' + name;
	
	var source_src = source_path;
	var target_local_src = target_local_path;
	var target_public_src = target_public_path;
	
	if ( pkg_json.src ) {
		source_src = path.resolveLocal(source_src, pkg_json.src);
		target_local_src = path.resolveLocal(target_local_src, pkg_json.src);
		target_public_src = path.resolveLocal(target_public_src, pkg_json.src);
	}
	
	self.m_cur_pkg_name             = name;
	self.m_cur_pkg_source_src       = source_src;
	self.m_cur_pkg_target_local_src = target_local_src;
	self.m_cur_pkg_target_public_src= target_public_src;
	self.m_cur_pkg_json             = pkg_json;
	self.m_cur_pkg_no_syntax_preprocess = !!pkg_json.no_syntax_preprocess;
	self.m_cur_pkg_qgr_syntax      = !!pkg_json.qgrSyntax;
	self.m_cur_pkg_files            = {};
	self.m_cur_pkg_pkg_files        = {};
	self.m_cur_pkg_skip_file        = get_skip_files(self, pkg_json, name);
	self.m_cur_pkg_detach_file      = get_detach_files(self, pkg_json, name);
	
	if ( pkg_json._Build ) { // 已经build过,直接拷贝到目标
		copy_pkg(self, pkg_json, source_path);
		return pkg.local_depe;
	}
	
	if ( self.minify == -1 ) { // 使用package.json定义
		// package.json 默认不启用 `minify`
		self.m_cur_pkg_enable_minify = 'minify' in pkg_json ? !!pkg_json.minify : false;
	} else {
		self.m_cur_pkg_enable_minify = !!self.minify;
	}

	fs.rm_r_sync(target_local_path);
	fs.rm_r_sync(target_public_path);
	fs.mkdir_p_sync(target_local_path);
	if ( !ignore_public ) {
		fs.mkdir_p_sync(target_public_path);
	}
	
	// each dir
	build_each_pkg_dir(self, '');
	
	var hash = new Hash();
	for (var i in self.m_cur_pkg_files) {  // 计算 version code
		hash.update_str(self.m_cur_pkg_files[i]);
	}
	
	pkg_json.versionCode = hash.digest();
	pkg_json.buildTime   = new Date().valueOf();
	pkg_json._Build       = true;
	delete pkg_json.versions;

	var cur_pkg_files = self.m_cur_pkg_files;
	var cur_pkg_pkg_files = self.m_cur_pkg_pkg_files;
	
	var local_depe = {};
	var public_depe = {};
	
	if ( !ignore_depe ) {
		// depe
		function solve_external_depe(pathname) {
			var paths = build_pkg(self, path.isAbsolute(pathname) ? 
														 pathname : source_path + '/' + pathname);
			local_depe[paths.absolute_path] = '';
			public_depe[paths.relative_path] = '';
		}
		if (Array.isArray(pkg_json.externDependencies)) {
			pkg_json.externDependencies.forEach(solve_external_depe);
		} else {
			for ( var i in pkg_json.externDependencies ) {
				solve_external_depe(i);
			}
		}
		//
		// TODO depe native ..
		//
	}

	pkg_json.externDependencies = local_depe;

	if ( ignore_public ) {
		var versions = { versions: cur_pkg_files };
		fs.writeFileSync(target_local_path + '/versions.json', JSON.stringify(versions, null, 2));
		fs.writeFileSync(target_local_path + '/package.json', JSON.stringify(pkg_json, null, 2));
	} else {
		
		var versions = { versions: cur_pkg_files, pkg_files: cur_pkg_pkg_files };
		
		pkg_json.externDependencies = public_depe;
		
		fs.writeFileSync(target_local_src + '/versions.json', JSON.stringify(versions, null, 2));
		fs.writeFileSync(target_local_src + '/package.json', JSON.stringify(pkg_json, null, 2));
		
		var pkg_files = ['package.json', 'versions.json'];
		for ( var i in cur_pkg_pkg_files ) {
			pkg_files.push('"' + i + '"');
		}
		new_zip(self, target_local_src, pkg_files, target_public_path + '/' + name + '.pkg');

		delete versions.pkg_files;

		pkg_json.externDependencies = local_depe;
		
		fs.rm_sync(target_local_src + '/versions.json');
		fs.rm_sync(target_local_src + '/package.json');
		fs.writeFileSync(target_local_path + '/versions.json', JSON.stringify(versions, null, 2));
		fs.writeFileSync(target_local_path + '/package.json', JSON.stringify(pkg_json, null, 2));

		pkg_json.externDependencies = public_depe;

		fs.writeFileSync(target_public_path + '/package.json', JSON.stringify(pkg_json, null, 2));

		if ( skip_install ) {
			var skip_install = path.resolveLocal(target_local_path, '../../skip_install');
			fs.mkdir_p_sync(skip_install);
			fs.rm_r_sync(skip_install + '/' + name);
			fs.renameSync(target_local_path, skip_install + '/' + name);
		}
	}
	
	return out;
}

function copy_file(self, source, target) {
	
	fs.mkdir_p_sync( path.dirname(target) ); // 先创建目录
	
	var rfd  = fs.openSync(source, 'r');
	var wfd  = fs.openSync(target, 'w');
	var size = 1024 * 100; // 100 kb
	var buff = new Buffer(size);
	var len  = 0;
	var hash = new Hash();
	
	do {
		len = fs.readSync(rfd, buff, 0, size, null);
		fs.writeSync(wfd, buff, 0, len, null);
		hash.update_buff_with_len(buff, len); // 更新hash
	} while (len == size);
	
	fs.closeSync(rfd);
	fs.closeSync(wfd);
	
	return { hash : hash.digest() };
}

function read_file_text(self, pathname) {
	var buff = fs.readFileSync(pathname);
	var hash = new Hash();
	hash.update_buff(buff);
	return {
		value: buff.toString('utf-8'),
		hash: hash.digest(),
	};
}

function build_file(self, pathname) {
	// 跳过文件
	for (var i = 0; i < self.m_cur_pkg_skip_file.length; i++) {
		var name = self.m_cur_pkg_skip_file[i];
		if ( pathname.indexOf(name) == 0 ) { // 跳过这个文件
			return;
		}
	}
	var source        = path.resolveLocal(self.m_cur_pkg_source_src, pathname);
	var target_local  = path.resolveLocal(self.m_cur_pkg_target_local_src, pathname);
	var target_public = path.resolveLocal(self.m_cur_pkg_target_public_src, pathname);
	var extname       = path.extname(pathname).toLowerCase();
	var data          = null;
	var is_detach     = false;
	
	for (var i = 0; i < self.m_cur_pkg_detach_file.length; i++) {
		var name = self.m_cur_pkg_detach_file[i];
		if (pathname.indexOf(name) == 0) {
			is_detach = true; // 分离这个文件
			break;
		}
	}
	
	switch (extname) {
		case '.js':
		case '.jsx':
			console_log(self, 'Out ', pathname);

			if ( self.m_cur_pkg_no_syntax_preprocess || 
					(extname == '.js' && !self.m_cur_pkg_qgr_syntax)) { 
				// 不进行jsa转换,直接使用原始代码
				data = read_file_text(self, source);
			} else {
				jsa_shell(source, source + 'c');
				data = read_file_text(self, source + 'c');
				fs.rm_sync(source + 'c');
				
				if ( self.m_cur_pkg_enable_minify ) {
					var minify = uglify.minify(data.value, {
						toplevel: true, 
						keep_fnames: false,
						mangle: { 
							toplevel: true, 
							reserved: [ '$' ], 
							keep_classnames: true,
						},
						output: { ascii_only: true } 
					});
					if ( minify.error ) {
						var err = minify.error;
						err = new SyntaxError(
							`${err.message}\n` +
							`line: ${err.line}, col: ${err.col}\n` +
							`filename: ${source}`
						);
						throw err;
					}
					data.value = minify.code;
				}
			}
			
			fs.mkdir_p_sync( path.dirname(target_local) ); // 先创建目录
			
			fs.writeFileSync(target_local, data.value, 'utf8');
			break;
		case '.keys':
			console_log(self, 'Out ', pathname);
			data = read_file_text(self, source);
			var keys_data = null;
			
			try {
				keys_data = keys.parse(data.value);
			} catch(err) {
				console.error('Parse keys file error: ' + source);
				throw err;
			}
			
			fs.mkdir_p_sync( path.dirname(target_local) ); // 先创建目录
			
			fs.writeFileSync(target_local, keys.stringify(keys_data), 'utf8');
			break;
		default:
			console_log(self, 'Copy', pathname);
			data = copy_file(self, source, target_local);
			break;
	}
	
	self.m_cur_pkg_files[pathname] = data.hash; // 记录文件 hash
	
	if ( is_detach ) { 
		fs.cp_sync(target_local, target_public);
	} else {  // add to .pkg public 
		self.m_cur_pkg_pkg_files[pathname] = data.hash;
	}
}

function build_each_pkg_dir(self, pathname) {
	
	var path2 = path.resolveLocal(self.m_cur_pkg_source_src, pathname);
	var ls = fs.ls_sync(path2);
	
	for (var i = 0; i < ls.length; i++) {
		var stat = ls[i];
		if (stat.name[0] != '.' || !self.ignore_hide) {
			var path3 = pathname ? pathname + '/' + stat.name : stat.name; 
			
			if ( stat.isFile() ) {
				build_file(self, path3);
			} else if ( stat.isDirectory() ) {
				build_each_pkg_dir(self, path3);
			}
		}
	}
}

function copy(self, source, target) {
	fs.cp_sync(source, target, { ignore_hide: self.ignore_hide });
}

function copy_pkg(self, pkg_json, source) {
	util.assert(pkg_json._Build, 'Error');
	
	var name = pkg_json.name;
	var target_local_path = self.m_target_local + '/' + name;
	var target_public_path = self.m_target_public + '/' + name;
	var pkg_path = source + '/' + name + '.pkg';

	// copy to ramote
	copy(source, target_public_path);
	// copy to local
	copy(source, target_local_path);
	
	if ( fs.existsSync(pkg_path) ) { // 有 .pkg
		// unzip .pkg
		fs.mkdir_p_sync(self.m_cur_pkg_target_local_src);
		unzip(self, pkg_path, self.m_cur_pkg_target_local_src);
		
		if ( self.m_cur_pkg_target_local_src != target_local_path ) { // src
			fs.fs.renameSync(self.m_cur_pkg_target_local_src + 
											 '/versions.json', target_local_path + '/versions.json');
			fs.rm_sync(self.m_cur_pkg_target_local_src + '/package.json');
		}
		fs.rm_sync(pkg_path);
	} else { // 没有.pkg文件
		new_zip(self, target_public_path, 'package.json versions.json', name + '.pkg');
		fs.rm_sync(target_public_path + '/versions.json');
	}
}

// 拷贝外部文件
function copy_outer_file(self, items) {
	for (var source in items) {
		var target = items[source] || source;
		console.log('Copy', source);
		fs.cp_sync(self.m_source + '/' + source, 
							 self.m_target_local + '/' + target, { ignore_hide: self.ignore_hide });
	}
}

function build_result(self) {
	var result = { };
	var ok = 0;
	for ( var name in self.m_output_pkgs ) {
		result[name] = self.m_output_pkgs[name].pkg_json;
		ok = 1;
	}
	if ( ok ) {
		fs.writeFileSync(self.m_target_public + '/packages.json', JSON.stringify(result, null, 2));
	} else {
		console.log('No package build');
	}
}

/**
 * @class QgrBuild
 */
var QgrBuild = util.class('QgrBuild', {
	
	m_source                    : '',
	m_target_local              : '',
	m_target_public             : '',
	m_cur_pkg_name              : '',
	m_cur_pkg_source_src        : '',
	m_cur_pkg_target_local_src  : '',
	m_cur_pkg_target_public_src : '',
	m_cur_pkg_json              : null,
	m_cur_pkg_no_syntax_preprocess : false,
	m_cur_pkg_qgr_syntax       : false,
	m_cur_pkg_files             : null,
	m_cur_pkg_pkg_files         : null,
	m_cur_pkg_detach_file       : null,
	m_cur_pkg_skip_file         : null,
	m_cur_pkg_enable_minify     : false,
	m_output_pkgs               : null,
	
	// public:
	
	ignore_hide: true, // 忽略隐藏文件
	minify: -1, // 缩小与混淆js代码，-1表示使用pkg.keys定义
	skip: null,// 跳过文件列表
	detach: null, // 分离文件列表
	
	/**
		* @constructor
		*/
	constructor: function (source, target) {
		var self = this;
		this.skip               = [];
		this.detach             = [];
		this.m_output_pkgs      = {};
		this.m_source           = path.resolveLocal(source);
		this.m_target_local     = path.resolveLocal(target, 'install');
		this.m_target_public    = path.resolveLocal(target, 'public');
		
		util.assert(fs.existsSync(this.m_source), 'Build source does not exist ,{0}', this.m_source);
	},
	
	/**
	 * action
	 */
	build: function() { 
		var self = this;
		var keys_path = self.m_source + '/proj.keys';

		fs.mkdir_p_sync(this.m_target_local);
		fs.mkdir_p_sync(this.m_target_public);

		if (!fs.existsSync(`${self.m_source}/.gitignore`)) {
			fs.writeFileSync(`${self.m_source}/.gitignore`, 'out\n');
		}

		if (fs.existsSync(`${self.m_source}/.editorconfig`)) {
			fs.writeFileSync(`${self.m_source}/.editorconfig`,
`
# top-most EditorConfig file  
root = true  
  
# all files  
[*]  
indent_style = tab  
indent_size = 2

`
			);
		}
		
		if ( !fs.existsSync(keys_path) ) { // No exists proj.keys file
			// build pkgs
			// scan each current target directory
			fs.ls_sync(self.m_source).forEach(function(stat) {
				if ( stat.name[0] != '.' && 
						 stat.isDirectory() && 
						 fs.existsSync( self.m_source + '/' + stat.name + '/package.json' )
				) {
					build_pkg(self, self.m_source + '/' + stat.name);
				}
			});
			build_result(self);

			return;
		}

		var keys_object = keys.parseFile( keys_path );
		var apps = [];

		for (var key in keys_object) {
			if (key == '@apps') {
				for (var name in keys_object['@apps']) {
					apps.push(name);
				}
			} else if (key == '@copy') {
				copy_outer_file(self, keys_object['@copy']);
			}
		}

		// npm install
		console.log(`Install dependencies ...`);
		var exists_package = fs.existsSync('package.json');
		var exists_node_modules = fs.existsSync('node_modules');
		if (exists_package)
			fs.renameSync('package.json', '.package.json.bk');
		if (exists_node_modules)
			fs.renameSync('node_modules', '.node_modules.bk');
		if (fs.existsSync('libs'))
			fs.renameSync('libs', 'node_modules');

		fs.writeFileSync('package.json', '{}');
		syscall(`npm install ${apps.join(' ')} --save=.`);

		apps.forEach(e=>fs.unlinkSync('node_modules/' + e)); // delete uselse file

		if (fs.existsSync('node_modules')) {
			if (fs.readdirSync('node_modules').length) {
				fs.renameSync('node_modules', 'libs');
			} else {
				fs.rmdirSync('node_modules');
			}
		}
		fs.rm_r_sync('package-lock.json');
		fs.rm_r_sync('package.json');

		if (exists_package)
			fs.renameSync('.package.json.bk', 'package.json');
		if (exists_node_modules)
			fs.renameSync('.node_modules.bk', 'node_modules');


		// build application pkgs

		var pkgs_path = self.m_source + '/libs';

		if ( fs.existsSync(pkgs_path) && fs.statSync(pkgs_path).isDirectory() ) {

			var target_local = this.m_target_local; // + '/pkgs';
			fs.mkdir_p_sync(target_local);

			fs.ls_sync(pkgs_path).forEach(function(stat) {
				var source = pkgs_path + '/' + stat.name;
				if ( stat.isDirectory() && fs.existsSync(source + '/package.json') ) {
					build_pkg1(self, source, target_local, self.m_target_public, false, true);
				}
			});
		}
		
		// build apps

		apps.forEach(e=>build_pkg(self, self.m_source + '/' + e));

		build_result(self);
	},

	/**
	 * @func initialize() init project directory and add examples
	 */
	initialize: function() {
		var project_name = path.basename(process.cwd()) || 'qgrproj';
		var proj_keys = this.m_source + '/proj.keys';
		var proj = { '@projectName': project_name };
		var default_modules = paths.default_modules;

		if ( default_modules && default_modules.length ) {
			var pkgs_dirname = this.m_source + '/libs';
			fs.mkdir_p_sync(pkgs_dirname); // create pkgs dir
			// copy default pkgs
			default_modules.forEach(function(pkg) { 
				var pathname = pkgs_dirname + '/' + path.basename(pkg);
				if ( !fs.existsSync(pathname) ) { // if no exists then copy
					fs.cp_sync(pkg, pathname); // copy pkgs
				}
			});
		}

		if (fs.existsSync(proj_keys)) { // 如果当前目录存在proj.keys文件附加到当前
			proj = util.assign(proj, keys.parseFile(proj_keys));
		} else {
			proj['@apps'] = {};

			if (!fs.existsSync(project_name)) {
				var json = {
					name: project_name,
					appName: project_name,
					id: `org.qgr.${project_name}`,
					main: 'index.jsx',
					version: '1.0.0',
					qgrSyntax: true,
				};
				fs.mkdirSync(project_name);
				fs.writeFileSync(project_name + '/package.json', JSON.stringify(json, null, 2));
				fs.writeFileSync(project_name + '/index.jsx', 
`
import { GUIApplication, Root, Indep } from 'qgr';

new GUIApplication().start(
	<Root>
		<Indep align="center">Hello world</Indep>
	</Root>
);

`);
			}
			if (!fs.existsSync('examples')) { // copy examples pkg
				fs.cp_sync(paths.examples, this.m_source + '/examples');
			}
			proj['@projectName'] = project_name;

			if (fs.existsSync('examples/package.json')) {
				proj['@apps']['examples'] = '';
			}
			if (fs.existsSync(project_name + '/package.json')) {
				proj['@apps'][project_name] = '';
			}
		}
		
		// write new proj.keys
		fs.writeFileSync(proj_keys, keys.stringify(proj));
	},
	
});

exports.QgrBuild = QgrBuild;
