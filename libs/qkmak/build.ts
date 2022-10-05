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

import util from 'somes';
import * as fs from 'somes/fs';
import * as child_process from 'child_process';
import keys from 'somes/keys';
import path from 'somes/path';
import paths from './paths';
import { exec } from 'somes/syscall';

const uglify = require('./uglify');

const base64_chars =
	'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-'.split('');

const init_code = `
import { Application, Root, Indep, Text, _CVD } from 'quark';

new Application({ multisample: 4 }).start(
	<Root>
		<Indep align="center" backgroundColor="#f00">
			<Text value="Hello world" />
		</Indep>
	</Root>
);

`;

const init_code2 = `

// import utils from 'somes';

console.log('When the package has only one file, TSC cannot be compiled. This should be a bug of TSC');

`;

const init_editorconfig  = `
# top-most EditorConfig file  
root = true  
  
# all files  
[*]  
indent_style = tab  
indent_size = 2

`;

export const native_source = [
	'.c',
	'.cc',
	'.cpp',
	'.cxx',
	'.m',
	'.mm',
	'.s', 
	'.swift',
];

export const native_header = [
	'.h',
	'.hpp',
	'.hxx',
];

const skip_files = native_source.concat(native_header, [
	'.gyp',
	'.gypi',
]);

const init_tsconfig = {
	"compileOnSave": true,
	"compilerOptions": {
		"module": "commonjs",
		"target": "ES2018",
		"moduleResolution": "node",
		"sourceMap": false,
		"outDir": "out",
		"rootDir": ".",
		"baseUrl": ".",
		"declaration": true,
		"alwaysStrict": true,
		"allowJs": true,
		"checkJs": false,
		"strict": true,
		"noImplicitAny": true,
		"noImplicitThis": true,
		"strictNullChecks": true,
		"strictPropertyInitialization": false,
		"emitDecoratorMetadata": false,
		"experimentalDecorators": true,
		"removeComments": true,
		"jsx": "react",
		"jsxFactory": "_CVD",
		// "typeRoots" : ["../libs"],
		// "types" : ["node", "lodash", "express"]
	},
	"include": [
		"**/*",
	],
	"exclude": [
		"out",
		"var",
		".git",
		".svn",
		"node_modules",
		"_.js",
	]
};

function resolveLocal(...args: string[]) {
	return path.fallbackPath(path.resolve(...args));
}

function exec_cmd(cmd: string) {
	var r = child_process.spawnSync('sh', ['-c', cmd]);
	if (r.status != 0) {
		if (r.stdout.length) {
			console.log(r.stdout + '');
		}
		if (r.stderr.length) {
			console.error(r.stderr + '');
		}
		process.exit(0);
	} else {
		var rv = [];
		if (r.stdout.length) {
			rv.push(r.stdout);
		}
		if (r.stderr.length) {
			rv.push(r.stderr);
		}
		return rv.join('\n');
	}
}

function parse_json_file(filename: string, strict?: boolean) {
	try {
		var str = fs.readFileSync(filename, 'utf-8');
		if (strict) {
			return JSON.parse(str);
		} else {
			return eval('(\n' + str + '\n)');
		}
	} catch (err) {
		err.message = filename + ': ' + err.message;
		throw err;
	}
}

function new_zip(cwd: string, sources: string[], target: string) {
	console.log('Out ', path.basename(target));
	exec_cmd('cd ' + cwd + '; rm -r ' + target + '; zip ' + target + ' ' + sources.join(' '));
}

function unzip(source: string, target: string) {
	exec_cmd('cd ' + target + '; unzip ' + source);
}

function copy_file(source: string, target: string) {
	
	fs.mkdirpSync( path.dirname(target) ); // 先创建目录

	var rfd  = fs.openSync(source, 'r');
	var wfd  = fs.openSync(target, 'w');
	var size = 1024 * 100; // 100 kb
	var buff = Buffer.alloc(size);
	var len  = 0;
	var hash = new Hash();
	
	do {
		len = fs.readSync(rfd, buff, 0, size, null);
		fs.writeSync(wfd, buff, 0, len, null);
		hash.update_buff_with_len(buff, len); // 更新hash
	} while (len == size);
	
	fs.closeSync(rfd);
	fs.closeSync(wfd);
	
	return hash.digest();
}

function read_file_text(pathname: string) {
	var buff = fs.readFileSync(pathname);
	var hash = new Hash();
	hash.update_buff(buff);
	return {
		value: buff.toString('utf-8'),
		hash: hash.digest(),
	};
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
	detach?: string | string[];
	skip?: string | string[];
	skipInstall?: boolean;
	minify?: boolean;
}

type PkgJson = PackageJson;

class Hash {
	
	hash = 5381;
	
	update_str(input: string) {
		var hash = this.hash;
		for (var i = input.length - 1; i > -1; i--) {
			hash += (hash << 5) + input.charCodeAt(i);
		}
		this.hash = hash;
	}
	
	update_buff(input: Buffer) {
		var hash = this.hash;
		for (var i = input.length - 1; i > -1; i--) {
			hash += (hash << 5) + input[i];
		}
		this.hash = hash;
	}
	
	update_buff_with_len(input: Buffer, len: number) {
		var hash = this.hash;
		for (var i = len - 1; i > -1; i--) {
			hash += (hash << 5) + input[i];
		}
		this.hash = hash;
	}
	
	digest() {
		var value = this.hash & 0x7FFFFFFF;
		var retValue = '';
		do {
			retValue += base64_chars[value & 0x3F];
		}
		while ( value >>= 6 );
		return retValue;
	}
}

class Package {
	private m_output_name       = '';
	private m_source            = '';
	private m_target_local      = '';
	private m_target_public     = '';
	private m_versions: Dict<string> = {};
	private m_detach_file: string[] = [];
	private m_skip_file: string[] = [];
	private m_enable_minify     = false;
	private m_tsconfig_outDir   = '';
	private m_host: QuarkBuild;

	readonly json: PkgJson;

	private _console_log(tag: string, pathname: string, desc?: string) {
		console.log(tag, this.m_output_name + '/' + pathname, desc || '');
	}

	constructor(host: QuarkBuild, source_path: string, outputName: string, json: PkgJson) {
		this.m_host = host;
		this.m_source = source_path;
		this.json = json;
		this.m_output_name      = outputName;
		this.m_target_local     = this.m_host.target_local + '/' + outputName;
		this.m_target_public    = this.m_host.target_public + '/' + outputName;
		this.m_skip_file        = this._get_skip_files(this.json, outputName);
		this.m_detach_file      = this._get_detach_files(this.json, outputName);
		host.outputs[outputName] = this;
	}

	// 获取跳过文件列表
	// "name" pkg 名称
	private _get_skip_files(pkg_json: PkgJson, name: string) {
		var self = this;
		var rev: string[] = [];

		if (pkg_json.skip) {
			if (Array.isArray(pkg_json.skip)) {
				rev = pkg_json.skip;
			} else {
				rev = [ String(pkg_json.skip) ];
			}
			delete pkg_json.skip;
		}

		rev.push('tsconfig.json');
		rev.push('binding.gyp');
		rev.push('node_modules');
		rev.push('out');
		rev.push('versions.json');

		var reg = new RegExp('^:?' + name + '$');
		self.m_host.skip.forEach(function (src) {
			var ls = src.split('/');
			if (reg.test(ls.shift() as string) && ls.length) {
				rev.push(ls.join('/'));
			}
		});

		return rev;
	}

	// 获取分离文件列表
	private _get_detach_files(pkg_json: PkgJson, name: string) {
		var self = this;
		var rev: string[] = [];
		
		if (pkg_json.detach) {
			if (Array.isArray(pkg_json.detach)) {
				rev = pkg_json.detach;
			} else {
				rev = [ String(pkg_json.detach) ];
			}
			delete pkg_json.detach;
		}
		
		var reg = new RegExp('^:?' + name + '$');
		self.m_host.detach.forEach(function (src) {
			var ls = src.split('/');
			if (reg.test(ls.shift() as string) && ls.length) {
				rev.push(ls.join('/'));
			}
		});
		return rev;
	}

	build() {
		var self = this;
		var source_path = self.m_source;
		var pkg_json = self.json;

		if ( pkg_json.hash ) { // 已经build过,直接拷贝到目标
			self._copy_pkg(pkg_json, source_path);
			return;
		}

		var target_local_path = self.m_target_local;
		var target_public_path = self.m_target_public;

		if ( self.m_host.minify == -1 ) { // 使用package.json定义
			// package.json 默认不启用 `minify`
			self.m_enable_minify = 'minify' in pkg_json ? !!pkg_json.minify : false;
		} else {
			self.m_enable_minify = !!self.m_host.minify;
		}

		fs.removerSync(target_local_path);
		fs.removerSync(target_public_path);
		fs.mkdirpSync(target_local_path);
		fs.mkdirpSync(target_public_path);

		// build tsc
		if (fs.existsSync(source_path + '/tsconfig.json')) {
			self.m_tsconfig_outDir = resolveLocal(self.m_host.target_local, '../tsc', self.m_output_name);
			exec_cmd(`cd ${source_path} && tsc --outDir ${self.m_tsconfig_outDir}`);
		}

		// each dir
		self._build_each_pkg_dir('', '');

		var hash = new Hash();
		for (var i in self.m_versions) {  // 计算 version code
			hash.update_str(self.m_versions[i]);
		}

		pkg_json.hash = hash.digest();

		var cur_pkg_versions = self.m_versions;
		var versions = { versions: cur_pkg_versions };
		delete pkg_json.skipInstall;

		fs.writeFileSync(target_local_path + '/versions.json', JSON.stringify(versions, null, 2));
		fs.writeFileSync(target_local_path + '/package.json', JSON.stringify(pkg_json, null, 2)); // rewrite package.json
		fs.writeFileSync(target_public_path + '/package.json', JSON.stringify(pkg_json, null, 2)); // rewrite package.json

		var pkg_files = ['versions.json'];
		for ( var i in versions.versions ) {
			if (versions.versions[i].charAt(0) != '.')
				pkg_files.push('"' + i + '"');
		}
		new_zip(target_local_path, pkg_files, target_public_path + '/' + pkg_json.name + '.pkg');
	}

	private _copy_js(source: string, target_local: string) {
		var self = this;
		var data = read_file_text(source);

		if ( self.m_enable_minify ) {
			var minify = uglify.minify(data.value, {
				toplevel: true,
				keep_fnames: false,
				mangle: {
					toplevel: true,
					reserved: [ '$' ],
					keep_classnames: true,
				},
				output: { ascii_only: true },
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

			var hash = new Hash();
			hash.update_str(data.value);
			data.hash = hash.digest();
		}

		fs.mkdirpSync( path.dirname(target_local) ); // 先创建目录

		fs.writeFileSync(target_local, data.value, 'utf8');

		return data.hash;
	}

	private _write_string(pathname: string, content: string) {
		var self = this;
		var target_local  = resolveLocal(self.m_target_local, pathname);
		// var target_public = resolveLocal(self.m_target_public, pathname);
		fs.mkdirpSync( path.dirname(target_local) ); // 先创建目录
		fs.writeFileSync(target_local, content, 'utf8');
		var hash = new Hash();
		hash.update_str(content);
		// fs.cp_sync(target_local, target_public);
		self.m_versions[pathname] = hash.digest(); // 记录文件 hash
	}

	private _build_file(pathname: string) {
		var self = this;
		// 跳过文件
		for (var i = 0; i < self.m_skip_file.length; i++) {
			var name = self.m_skip_file[i];
			if ( pathname.indexOf(name) == 0 ) { // 跳过这个文件
				self._console_log('Skip', pathname);
				return;
			}
		}
		var source        = resolveLocal(self.m_source, pathname);
		var target_local  = resolveLocal(self.m_target_local, pathname);
		var target_public = resolveLocal(self.m_target_public, pathname);
		var extname       = path.extname(pathname).toLowerCase();
		var is_detach     = false;
		var hash          = '';

		if (skip_files.indexOf(extname) != -1) {
			return; // skip native file
		}

		for (var i = 0; i < self.m_detach_file.length; i++) {
			var name = self.m_detach_file[i];
			if (pathname.indexOf(name) === 0) {
				is_detach = true; // 分离这个文件
				break;
			}
		}

		switch (extname) {
			case '.js':
				self._console_log('Out ', pathname);
				if (self.m_tsconfig_outDir) {
					if (fs.existsSync(self.m_tsconfig_outDir + '/' + pathname))
						source = self.m_tsconfig_outDir + '/' + pathname;
				}
				hash = self._copy_js(source, target_local);
				break;
			case '.ts':
			case '.tsx':
			case '.jsx':
				if (pathname.substr(-2 - extname.length, 2) == '.d') { // typescript define
					self._console_log('Copy', pathname);
					hash = copy_file(source, target_local);
				} else if (self.m_tsconfig_outDir) {
					pathname = pathname.substr(0,  pathname.length - extname.length) + '.js';
					target_local = resolveLocal(self.m_target_local, pathname);
					target_public = resolveLocal(self.m_target_public, pathname);
					hash = self._copy_js(self.m_tsconfig_outDir + '/' + pathname, target_local);
				} else {
					self._console_log('Ignore', pathname, 'No tsconfig.json');
					return;
				}
				break;
			case '.keys':
				self._console_log('Out ', pathname);
				var {hash,value} = read_file_text(source);
				var keys_data = null;

				try {
					keys_data = keys.parse(value);
				} catch(err) {
					console.error('Parse keys file error: ' + source);
					throw err;
				}
				fs.mkdirpSync( path.dirname(target_local) ); // 先创建目录
				fs.writeFileSync(target_local, keys.stringify(keys_data), 'utf8');
				break;
			default:
				self._console_log('Copy', pathname);
				hash = copy_file(source, target_local);
				break;
		}

		if ( is_detach ) {
			fs.cp_sync(target_local, target_public);
			hash = '.' + hash; // Separate files with "." before hash
		}

		self.m_versions[pathname] = hash; // 记录文件 hash
	}

	private _build_each_pkg_dir(pathname: string, basename: string) {
		var self = this;
		var dir = resolveLocal(self.m_source, pathname);

		if (self.m_tsconfig_outDir == dir) { // skip ts out dir
			if (self.m_tsconfig_outDir != self.m_source) { // no skip root source
				return;
			}
		}

		if (basename == 'node_modules') {
			var pkgs: Dict<PkgJson> = {};
			var ok = false;
			for (var stat of fs.listSync(dir)) {
				var pkg_path = dir + '/' + stat.name;
				if (stat.isDirectory() && fs.existsSync( pkg_path + '/package.json')) {
					var pkg = self.m_host.solve(pkg_path, true) as Package;
					var symlink = path.relative(`${self.m_target_local}/${pathname}`, `${self.m_host.target_local}/${pkg.m_output_name}`);
					self._write_string(pathname + '/' + pkg.json.name + '.link', symlink);
					pkgs[pkg.json.name] = pkg.json;
					pkg.json.symlink = symlink;
					ok = true;
				}
			}
			if (ok)
				self._write_string(pathname + '/packages.json', JSON.stringify(pkgs, null, 2));
		} else {
			for (var stat of fs.listSync(dir)) {
				if (stat.name[0] != '.' || !self.m_host.ignore_hide) {
					var basename = stat.name;
					let path = pathname ? pathname + '/' + basename : basename; 
					if ( stat.isFile() ) {
						self._build_file(path);
					} else if ( stat.isDirectory() ) {
						self._build_each_pkg_dir(path, basename);
					}
				}
			}
		}
	}

	private _copy(source: string, target: string) {
		fs.cp_sync(source, target, { ignore_hide: this.m_host.ignore_hide });
	}

	private _copy_pkg(pkg_json: PkgJson, source: string) {
		var self = this;
		util.assert(pkg_json.hash, 'Error');

		var name = pkg_json.name;
		var target_local_path = self.m_target_local + '/' + name;
		var target_public_path = self.m_target_public + '/' + name;
		var pkg_path = source + '/' + name + '.pkg';

		// copy to ramote
		self._copy(source, target_public_path);
		// copy to local
		self._copy(source, target_local_path);

		if ( fs.existsSync(pkg_path) ) { // local 有.pkg
			// unzip .pkg
			unzip(pkg_path, target_local_path);
			fs.removerSync(target_local_path + '/' + name + '.pkg');
		} else { // public 没有.pkg文件
			var versions = parse_json_file(source + '/versions.json');
			var pkg_files = ['versions.json'];
			for ( var i in versions.versions ) {
				if (versions.versions[i].charAt(0) != '.') {
					pkg_files.push('"' + i + '"');
					fs.removerSync(target_public_path + '/' + i);
				}
			}
			new_zip(source, pkg_files, target_public_path + '/' + name + '.pkg');
			fs.removerSync(target_public_path + '/versions.json');
			fs.cp_sync(source + '/package.json', target_public_path + '/package.json');
		}
	}

}

export default class QuarkBuild {
	
	readonly source: string;
	readonly target_local: string;
	readonly target_public: string;
	readonly outputs: Dict<Package>= {};

	ignore_hide = true; // 忽略隐藏文件
	minify = -1; // 缩小与混淆js代码，-1表示使用package.json定义
	skip: string[] = [];// 跳过文件列表
	detach: string[] = []; // 分离文件列表

	constructor(source: string, target: string) {
		this.source           = resolveLocal(source);
		this.target_local     = resolveLocal(target, 'install');
		this.target_public    = resolveLocal(target, 'public');
		
		util.assert(fs.existsSync(this.source), `Build source does not exist ,${this.source}`);
		util.assert(fs.statSync(this.source).isDirectory());
	}

	private _copy_outer_file(items: Dict<string>) {
		var self = this;
		for (var source in items) {
			var target = items[source] || source;
			console.log('Copy', source);
			fs.cp_sync(self.source + '/' + source, 
								 self.target_local + '/' + target, { ignore_hide: self.ignore_hide });
		}
	}

	solve(pathname: string, hasFullname?: boolean) {
		var self = this;
		var source_path = resolveLocal(pathname);

		// ignore network pkg 
		if ( /^https?:\/\//i.test(source_path) ) { 
			return null;
		}

		var pkg_json: PkgJson | null = null;
		var __ = ()=>{
			if (!pkg_json) {
				pkg_json = parse_json_file(source_path + '/package.json') as PkgJson;
			}
			return pkg_json;
		};

		var outputName = hasFullname ? __().name + '@' + __().version: path.basename(source_path);
		var pkg = self.outputs[outputName];
		if ( !pkg ) { // Already complete
			pkg = new Package(this, source_path, outputName, __());
			pkg.build();
		}

		return pkg;
	}

	private _build_result() {
		var self = this;
		var result: Dict<PkgJson> = {};
		var ok = 0;
		for ( var name in self.outputs ) {
			result[name] = self.outputs[name].json;
			ok = 1;
		}
		if ( ok ) {
			fs.writeFileSync(self.target_public + '/packages.json', JSON.stringify(result, null, 2));
			console.log('build ok');
		} else {
			console.log('No package build');
		}
	}

	async install_depe() {
		var self = this;
		var keys_path = self.source + '/proj.keys';

		if ( !fs.existsSync(keys_path) )
			return [];

		var proj = keys.parseFile( keys_path );
		var apps = [];

		for (var key in proj) {
			if (key == '@apps') {
				for (var name in proj['@apps']) {
					apps.push(name);
				}
			}
		}

		try {
			// npm install
			console.log(`Install dependencies ...`);
			fs.writeFileSync('package.json', '{}');

			process.stdin.resume();

			var r = await exec(`npm install ${apps.map(e=>'./'+e).join(' ')} --save=. --only=prod --ignore-scripts`, {
				stdout: process.stdout,
				stderr: process.stderr, stdin: process.stdin,
			});
			process.stdin.pause();

			util.assert(r.code === 0);

			if (!fs.existsSync('node_modules/@types/quark')) { // copy @types
				fs.cp_sync(paths.types, this.source + '/node_modules/@types');
			}
		} finally {
			fs.removerSync('package-lock.json');
			fs.removerSync('package.json');
			apps.map(e=>'node_modules/' + e)
				.forEach(e => fs.existsSync(e)&&fs.unlinkSync(e)); // delete uselse file
		}

		return apps;
	}

	async build() {
		var self = this;

		fs.mkdirpSync(this.target_local);
		fs.mkdirpSync(this.target_public);

		var keys_path = self.source + '/proj.keys';

		if ( !fs.existsSync(keys_path) ) { // No exists proj.keys file
			// build pkgs
			// scan each current target directory
			fs.listSync(self.source).forEach(function(stat) {
				if ( stat.name[0] != '.' && 
						 stat.isDirectory() && 
						 fs.existsSync( self.source + '/' + stat.name + '/package.json' )
				) {
					self.solve(self.source + '/' + stat.name);
				}
			});
			self._build_result();

			return;
		}

		var keys_object = keys.parseFile( keys_path );
		for (var key in keys.parseFile( keys_path )) {
			if (key == '@copy') {
				self._copy_outer_file(keys_object['@copy']);
			}
		}

		var apps = await this.install_depe();

		// build application node_modules

		var node_modules = self.source + '/node_modules';

		if ( fs.existsSync(node_modules) && fs.statSync(node_modules).isDirectory() ) {
			fs.listSync(node_modules).forEach(function(stat) {
				var source = node_modules + '/' + stat.name;
				if ( stat.isDirectory() && fs.existsSync(source + '/package.json') ) {
					self.solve(source);
				}
			});
		}

		// build apps
		for (var app of apps) {
			self.solve(self.source + '/' + app);
		}

		self._build_result();
	}

	/**
	 * @func initialize() init project directory and add examples
	 */
	initialize() {
		var self = this;
		var project_name = path.basename(process.cwd()) || 'noprojroj';
		var proj_keys = this.source + '/proj.keys';
		var proj: Dict = { '@projectName': project_name };
		var default_modules = paths.default_modules;

		if ( default_modules && default_modules.length ) {
			var pkgs_dirname = this.source + '/node_modules';
			fs.mkdir_p_sync(pkgs_dirname); // create pkgs dir
			// copy default pkgs
			default_modules.forEach(function(pkg) { 
				var pathname = pkgs_dirname + '/' + path.basename(pkg);
				if ( !fs.existsSync(pathname) ) { // if no exists then copy
					fs.cp_sync(pkg, pathname); // copy pkgs
				}
			});
		}

		if (!fs.existsSync(`${self.source}/.gitignore`)) {
			fs.writeFileSync(`${self.source}/.gitignore`, ['out', 'node_modules'].join('\n'));
		}
		if (!fs.existsSync(`${self.source}/.editorconfig`)) {
			fs.writeFileSync(`${self.source}/.editorconfig`, init_editorconfig);
		}

		if (fs.existsSync(proj_keys)) { // 如果当前目录存在proj.keys文件附加到当前
			proj = Object.assign(proj, keys.parseFile(proj_keys));
		} else {
			proj['@apps'] = {};

			if (!fs.existsSync(project_name)) {
				var json = {
					name: project_name,
					app: project_name,
					id: `org.quark.${project_name}`,
					main: 'index.js',
					types: 'index.d.ts',
					version: '1.0.0',
				};
				init_tsconfig.compilerOptions.outDir = 'out/' + project_name;
				fs.mkdirSync(project_name);
				fs.writeFileSync(project_name + '/package.json', JSON.stringify(json, null, 2));
				fs.writeFileSync(project_name + '/index.tsx', init_code);
				fs.writeFileSync(project_name + '/test.ts', init_code2);
				fs.writeFileSync(project_name + '/tsconfig.json', JSON.stringify(init_tsconfig, null, 2));
			}

			if (!fs.existsSync('examples')) { // copy examples pkg
				fs.cp_sync(paths.examples, this.source + '/examples');
			}
			proj['@projectName'] = project_name;

			if (fs.existsSync('examples/package.json')) {
				proj['@apps']['examples'] = '';
			}

			if (!fs.existsSync('node_modules/@types/quark')) { // copy @types
				fs.cp_sync(paths.types, this.source + '/node_modules/@types');
			}

			if (fs.existsSync(project_name + '/package.json')) {
				proj['@apps'][project_name] = '';
			}
		}

		// write new proj.keys
		fs.writeFileSync(proj_keys, keys.stringify(proj));
	}
	
}