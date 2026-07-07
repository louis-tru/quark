/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

var fs = require('qktool/node/fs');
var path = require('path');
var child_process = require('child_process');

var COMPILE_SOURCE_EXTENSIONS = new Set([
	'.c', '.cc', '.cpp', '.cxx', '.m', '.mm',
]);

var COMPILE_SOURCE_SKIP_DIRS = new Set([
	'.git', '.svn', 'deps', 'node_modules', 'out',
]);

var COMPILE_SOURCE_SKIP_PATHS = new Set([
	'tools/linux', 'tools/ndk', 'tools/pkgs',
]);

function collect_compile_sources(root, dir, out) {
	for (var name of fs.readdirSync(dir)) {
		if (COMPILE_SOURCE_SKIP_DIRS.has(name)) {
			continue;
		}
		var file = path.join(dir, name);
		var stat = fs.statSync(file);
		if (stat.isDirectory()) {
			var relative = path.relative(root, file);
			if (!COMPILE_SOURCE_SKIP_PATHS.has(relative)) {
				collect_compile_sources(root, file, out);
			}
		} else if (COMPILE_SOURCE_EXTENSIONS.has(path.extname(name))) {
			out.push(file);
		}
	}
}

function write_compile_commands(root, ideDir) {
	var flagsFile = path.join(ideDir, 'compile_flags.txt');
	if (!fs.existsSync(flagsFile)) {
		return;
	}
	var flags = fs.readFileSync(flagsFile).toString()
		.split(/\r?\n/)
		.map(e => e.trim())
		.filter(Boolean);
	var sources = [];
	collect_compile_sources(root, root, sources);
	sources.sort();

	var commands = sources.map(file => ({
		directory: ideDir,
		file,
		arguments: ['clang++'].concat(flags, ['-c', file]),
	}));
	fs.writeFileSync(
		path.join(ideDir, 'compile_commands.json'),
		JSON.stringify(commands, null, 2) + '\n'
	);
}

function find_metal_lsp() {
	var home = process.env.HOME;
	var extensionRoots = [];

	if (process.env.VSCODE_EXTENSIONS) {
		extensionRoots.push(process.env.VSCODE_EXTENSIONS);
	}
	if (home) {
		extensionRoots.push(
			path.join(home, '.vscode/extensions'),
			path.join(home, '.vscode-insiders/extensions'),
			path.join(home, '.cursor/extensions')
		);
	}

	for (var root of extensionRoots) {
		if (!fs.existsSync(root)) {
			continue;
		}
		var names = fs.readdirSync(root)
			.filter(e => e.indexOf('rayanhtt.metal-lsp-') == 0)
			.sort()
			.reverse();
		for (var name of names) {
			var extension = path.join(root, name);
			var compatHeader = path.join(extension, 'headers/metal_compat.h');
			var stubs = path.join(extension, 'stubs');
			if (fs.existsSync(compatHeader) && fs.existsSync(stubs)) {
				return { compatHeader, stubs };
			}
		}
	}
}

function apple_sdk_path(sdk) {
	try {
		return child_process.execFileSync('xcrun', ['--sdk', sdk, '--show-sdk-path'], {
			encoding: 'utf8',
			stdio: ['ignore', 'pipe', 'ignore'],
		}).trim();
	} catch (err) {
		return '';
	}
}

function clangd_platform_flags(os) {
	var sdk = '';
	var define = '';
	if (os == 'ios') {
		sdk = apple_sdk_path('iphoneos');
		define = '-DQk_iOS=1';
	} else if (os == 'mac') {
		sdk = apple_sdk_path('macosx');
		define = '-DQk_MacOS=1';
	}

	var flags = [];
	if (define) {
		flags.push(define);
	}
	if (sdk) {
		flags.push('-isysroot', sdk);
	}
	return flags.map(e => `    - ${e}`).join('\n');
}

function fill_clangd_template(content, root, opts) {
	var metalLsp = find_metal_lsp();
	content = content.replace(/\{\{QK_IDE_DIR\}\}/g, path.join(root, '.ide'));
	content = content.replace(/\{\{QK_CLANGD_PLATFORM_FLAGS\}\}/g,
		clangd_platform_flags(opts && opts.os)
	);
	if (metalLsp) {
		return content
			.replace(/\{\{METAL_LSP_COMPAT_HEADER\}\}/g, metalLsp.compatHeader)
			.replace(/\{\{METAL_LSP_STUBS\}\}/g, metalLsp.stubs)
			.replace(/^# \{\{METAL_LSP_(BEGIN|END)\}\}\n?/gm, '');
	}
	return content.replace(
		/^# \{\{METAL_LSP_BEGIN\}\}[\s\S]*?^# \{\{METAL_LSP_END\}\}\n?/m,
		''
	);
}

function write(root, opts = {}) {
	root = path.resolve(root);
	var ideDir = path.join(root, '.ide');

	var lldbTemplate = path.join(ideDir, 'lldbinit-Xcode');
	var lldbScript = path.join(ideDir, 'qk_lldb.py');
	if (fs.existsSync(lldbTemplate) && fs.existsSync(lldbScript)) {
		var lldbContent = fs.readFileSync(lldbTemplate).toString()
			.replace(/\{\{QK_LLDB_PY\}\}/g, lldbScript);
		fs.writeFileSync(path.join(root, '.lldbinit-Xcode'), lldbContent);
	}

	var clangdTemplate = path.join(ideDir, 'clangd');
	if (fs.existsSync(clangdTemplate)) {
		var clangdContent = fill_clangd_template(
			fs.readFileSync(clangdTemplate).toString(),
			root,
			opts
		);
		fs.writeFileSync(path.join(root, '.clangd'), clangdContent);
	}

	write_compile_commands(root, ideDir);
}

exports.write = write;
