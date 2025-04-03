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

var util = require('encark').default;
var fs = require('encark/fs');
var path = require('path');
var host_os = process.platform == 'darwin' ? 'mac': process.platform;
var host_arch = arch_format(process.arch);
var argument = require('encark/arguments');
var { syscall, execSync, exec } = require('encark/syscall');
var opts = argument.options;
var help_info = argument.helpInfo;
var def_opts = argument.defOpts;
var default_arch = host_arch || 'x86';
var android_api_level = 28; // android-9.0

def_opts(['help','h'], 0,       '-h, --help     print help info');
def_opts('v', 0,                '-v, --v        enable compile print info [{0}]');
def_opts('debug', 0,            '--debug        enable debug status [{0}]');
def_opts('os', host_os,         '--os=OS        system type ios/android/mac/linux/win [{0}]');
def_opts('arch', default_arch,  '--arch=CPU     cpu type options arm/arm64/mips/mips64/x86/x64 [{0}]');
def_opts('armv7', arm(),        '--armv7        enable armv7 [{0}]');
def_opts('armv7s', 0,           '--armv7s       enable armv7s form apple iphone [{0}]');
def_opts('arm-neon', arm(),     '--arm-neon     enable arm neno [{0}]');
def_opts('arm-vfp', opts.arch == 'arm64' ? 'vfpv4': 
										(opts.arch == 'arm' ? (opts.armv7 || opts.armv7s ? 'vfpv3' : 'vfpv2'): 'none'),
																'--arm-vfp=VAL  enable arm vfp options vfpv2/vfpv3/vfpv4/none [{0}]');
def_opts('arm-fpu', opts.arm_neon ? 'neon': opts.arm_vfp,
																'--arm-fpu=VAL  enable arm fpu [{0}]');
def_opts(['emulator', 'em'], 0, '--emulator,-em enable the emulator [{0}]');
def_opts('clang', isApple(opts.os) ? 1 : 0, 
																'--clang        enable clang compiler [{0}]');
def_opts('media', 'auto',       '--media        compile media [{0}]');
def_opts(['ndk-path','ndk'], process.env.ANDROID_NDK || '', 
																'--ndk-path     android NDK path [{0}]');
def_opts(['use-v8','v8'],'auto','--use-v8,-v8   force use javascript v8 library [{0}]');
def_opts('without-snapshot', 0, '--without-snapshot without snapshot for v8 [{0}]');
def_opts('without-ssl', 0,      '--without-ssl  build without SSL (disables crypto, https, inspector, etc.)');
def_opts('with-intl', 0,        '--with-intl    enable intl default:[{0}]');
def_opts('prefix', '/usr/local','--prefix       select the install prefix [{0}]');
def_opts('no-browser-globals', 0,'--no-browser-globals do not export browser globals like setTimeout, console, etc. [{0}]' +
																 '(This mode is not officially supported for regular applications)');
def_opts('without-visibility-hidden', 0, 
																'--without-visibility-hidden without visibility hidden [{0}]');
def_opts('suffix', '',          '--suffix=VAL Compile directory suffix [{0}]');
def_opts('without-embed-bitcode', 1,
																'--without-embed-bitcode disable apple embed-bitcode [{0}]');
def_opts(['use-gl', 'gl'], 1,   '--enable-gl,-gl use opengl backend [{0}]');
def_opts(['use-js', 'js'], 1,   '--use-js,-js enable javascript modules [{0}]');

function isApple(os) {
	return ['mac', 'ios'].indexOf(os) >= 0;
}

function arm() {
	return opts.arch.match(/^arm/) ? 1 : 0;
}

function arch_format(arch) {
	arch = arch == 'ia32' || arch == 'i386' ? 'x86' : arch;
	arch = arch == 'x86_64' || arch == 'ia64' ? 'x64' : arch;
	return arch;
}

function touch_file(pathnames) {
	if ( !Array.isArray(pathnames)) {
		pathnames = [ pathnames ];
	}
	pathnames.forEach(function(pathname) {
		if ( !fs.existsSync(pathname) ) {
			fs.mkdirpSync(path.dirname(pathname));
			fs.writeFileSync(pathname, '');
		}
	});
}

function touch_files(variables) {
	touch_file([
		'out/native-inl-js.cc',
		'out/native-ext-js.cc',
		'out/native-lib-js.cc',
		'out/native-font.cc',
		'out/native-glsl.cc',
	]);

	if (!fs.existsSync(`${__dirname}/../out/quark`)) {
		fs.removerSync(`${__dirname}/../out/quark`);
		fs.symlinkSync(path.resolve(`${__dirname}/../src`), path.resolve(`${__dirname}/../out/quark`));
	}

	if (['mac','ios'].indexOf(variables.os) == -1) {
		touch_file([
			`${variables.output}/obj.target/libquark.so`,
		]);
	}
	if (variables.os == 'android' && (variables.debug || variables.without_visibility_hidden)) {
		touch_file([
			`${variables.output}/obj.target/libquark_deps_test.so`,
		]);
	}
}

function configure_ffmpeg(opts, {variables}, clang, ff_install_dir) {
	var os = opts.os;
	var arch = opts.arch;
	var arch_name = variables.arch_name;
	var cmd = '';
	var source = __dirname + '/../deps/ffmpeg';

	var ff_opts = [
		`--prefix=${ff_install_dir}`,
		'--enable-small',
		// Program options:
		'--disable-programs',
		// Documentation options:
		'--disable-doc',
		// Component options:
		'--disable-avdevice',
		//'--disable-avcodec',
		//'--disable-avformat',
		//'--disable-swresample',
		//'--disable-swscale',
		'--disable-postproc',
		'--disable-avfilter',
		'--disable-pixelutils',
		// Individual component options:
		'--disable-encoders',
		'--disable-decoders',
		'--disable-hwaccels',
		'--disable-muxers',
		'--disable-demuxers',
		'--disable-parsers',
		'--disable-bsfs',
		'--disable-protocols',
		'--disable-devices',
		'--disable-filters',
		// enable decoders
		'--enable-decoder=aac',
		'--enable-decoder=aac_fixed',
		'--enable-decoder=aac_latm',
		'--enable-decoder=ac3',
		'--enable-decoder=ac3_fixed',
		'--enable-decoder=mp1',
		'--enable-decoder=mp2',
		'--enable-decoder=mp3',
		'--enable-decoder=dca',
		'--enable-decoder=mpeg1video',
		'--enable-decoder=mpeg2video',
		'--enable-decoder=mpeg4',
		'--enable-decoder=mpegvideo',
		'--enable-decoder=h263',
		'--enable-decoder=h264',
		'--enable-decoder=hevc',
		// enable parsers
		'--enable-parser=aac',
		'--enable-parser=aac_latm',
		'--enable-parser=ac3',
		'--enable-parser=dca',
		'--enable-parser=h263',
		'--enable-parser=h264',
		'--enable-parser=hevc',
		'--enable-parser=mpeg4video',
		'--enable-parser=mpegaudio',
		'--enable-parser=mpegvideo',
		// enable demuxers
		'--enable-demuxer=aac',
		'--enable-demuxer=ac3',
		'--enable-demuxer=h263',
		'--enable-demuxer=h264',
		'--enable-demuxer=hevc',
		'--enable-demuxer=hls',
		'--enable-demuxer=m4v',
		'--enable-demuxer=mlv',
		'--enable-demuxer=mov',
		'--enable-demuxer=mp3',
		'--enable-demuxer=matroska',
		'--enable-demuxer=webm_dash_manifest',
		'--enable-demuxer=mpegps',
		'--enable-demuxer=mpegts',
		'--enable-demuxer=mpegtsraw',
		'--enable-demuxer=mpegvideo',
		'--enable-demuxer=rtsp',
		'--enable-demuxer=rtp',
		// enable protocols
		'--enable-protocol=hls',
		'--enable-protocol=http',
		'--enable-protocol=file',
		'--enable-protocol=rtmp',
		'--enable-protocol=rtsp',
		'--enable-protocol=rtp',
		// '--enable-openssl',
		// others
		// `--extra-cflags="${path.resolve(__dirname, '../deps/openssl/openssl/include')}"`,
		// --extra-ldflags="-L/path/to/openssl/lib"
	];

	if (os == 'android') {
		cmd = `\
			./configure \
			--target-os=android \
			--arch=${arch} \
			--sysroot=${variables.build_sysroot} \
			--enable-cross-compile \
		`;
			// --enable-jni \
			// --enable-mediacodec \
		var cc = variables.cc;
		var cflags = '';
		var cflags = `-ffunction-sections -fdata-sections `;
		if ( !clang ) { // use gcc
			cflags += '-funswitch-loops ';
		} else if (arch == 'arm') { // clang and arm
			// cflags += '-mllvm -arm-assume-gnu ';
			cflags += '-no-integrated-as ';
		}
		cmd += `--cc='${cc} ${cflags} -march=${arch_name}' `;
		cmd += `--strip=${variables.strip} `;
	}
	else if ( os=='linux' ) {
		cmd = `\
			./configure \
			--target-os=linux \
			--arch=${arch} \
		`;
		if ( host_arch != arch ) {
			cmd += '--enable-cross-compile ';
		}
		var cc = variables.cc;
		var cflags = '-ffunction-sections -fdata-sections -funswitch-loops ';

		if ( arch == 'arm' || arch == 'arm64' ) {
			cmd += `--cc='${cc} ${cflags} -march=${arch_name}' `;
		} else {
			cmd += `--cc='${cc} ${cflags}' `;
		}
	}
	else if (os == 'ios') {
		cmd = `\
			./configure \
			--target-os=darwin \
			--arch=${arch} \
			--enable-cross-compile \
		`;

		// -fembed-bitcode-marker
		var f_embed_bitcode = opts.without_embed_bitcode ?  '' : '-fembed-bitcode';
		var cc = `clang -arch ${arch_name} ${f_embed_bitcode} `;

		if (variables.emulator) {
			cc += `-mios-simulator-version-min=${variables.version_min} `
		} else {
			cc += `-miphoneos-version-min=${variables.version_min} `
		}

		cmd += `--cc='${cc}' `;
		cmd += `--sysroot=${variables.build_sysroot} `;

		if (arch == 'arm') {
			var as = `${__dirname}/gas-preprocessor.pl -arch arm -as-type apple-clang -- ${cc}`;
			cmd += `--as='${as}' `;
		}
	} 
	else if (os == 'mac') {
		cmd = `\
			./configure \
			--target-os=darwin \
			--arch=${arch} \
		`;
		cmd += `--cc='clang -mmacosx-version-min=${variables.version_min}` +
					// ` -Wno-incompatible-function-pointer-types` +
					` -arch ${arch_name} ' `;
		cmd += '--sysroot=$(xcrun --sdk macosx --show-sdk-path) ';
	}
	
	if ( !cmd ) {
		return false;
	}

	if (os == 'android') {
		ff_opts.push('--enable-pic');
	}
	if (opts.debug) {
		ff_opts.push('--enable-debug=2');
	} else {
		ff_opts.push('--disable-logging');
		ff_opts.push('--disable-debug'); 
		ff_opts.push('--strip');
	}

	cmd += ff_opts.join(' ');

	// clean
	execSync(`cd deps/ffmpeg; make clean; find . -name *.o|xargs rm; `);
	syscall(`
		rm -rf ${variables.output}/obj.target/deps/ffmpeg/*;
		rm -rf ${ff_install_dir}; \
		rm -rf \
		${source}/compat/strtod.d \
		${source}/compat/strtod.o \
		${source}/.config \
		${source}/config.fate \
		${source}/config.log \
		${source}/config.mak \
		${source}/doc/config.texi \
		${source}/doc/examples/pc-uninstalled \
		${source}/libavcodec/libavcodec.pc \
		${source}/libavdevice/libavdevice.pc \
		${source}/libavfilter/libavfilter.pc \
		${source}/libavformat/libavformat.pc \
		${source}/libavutil/libavutil.pc \
		${source}/libswresample/libswresample.pc \
		${source}/libswscale/libswscale.pc \
	`);

	console.log('FFMpeg Configuration:\n');
	console.log(`export PATH=${__dirname}:${variables.build_bin}:$PATH`);
	console.log(cmd, '\n');

	var log = syscall(
		`export PATH=${__dirname}:${variables.build_bin}:$PATH; cd deps/ffmpeg; ${cmd};`
	);
	console.error(log.stderr.join('\n'));
	console.log(log.stdout.join('\n'));

	return true;
}

function bi(a) {
	return a ? 1 : 0;
}

function GetFlavor(params = {}) {
	// """Returns |params.flavor| if it's set, the system's default flavor else."""
	var flavors = {
		'cygwin': 'win',
		'win32': 'win',
		'darwin': 'mac',
	}

	if ('flavor' in params)
		return params['flavor']
	if (process.platform in flavors) 
		return flavors[sys.platform]
	if (process.platform.startsWith('sunos'))
		return 'solaris'
	if (process.platform.startsWith('freebsd'))
		return 'freebsd'
	if (process.platform.startsWith('openbsd'))
		return 'openbsd'
	if (process.platform.startsWith('netbsd'))
		return 'netbsd'
	if (process.platform.startsWith('aix'))
		return 'aix'

	return 'linux'
}

function is_use_dtrace() {
	var flavor = GetFlavor({ flavor: opts.os });
	return ['solaris', 'mac', 'linux', 'freebsd'].indexOf(flavor) > -1;
}

function get_OS(os) {
	return os;
}

async function exec2(cmd) {
	var r = await exec(cmd, {
		stdout: process.stdout,
		stderr: process.stderr, stdin: process.stdin,
	});
	if (r.code != 0) {
		throw Error.new(`Run cmd fail, "${cmd}", code = ${r.code}`);
	}
}

async function install_check(app, cmd) {
	console.log('check', app);

	if (app.indexOf('/') != -1) { // file
		if (fs.existsSync(app)) {
			return;
		}
	} else {
		if (execSync(`which ${app}`).code == 0) {
			return;
		}
	}

	if (host_os == 'linux') {
		// util.assert(execSync('which apt-get').code == 0, 'No command `apt-get`');
	} else if (host_os == 'mac') {
		// util.assert(execSync('which brew').code == 0, 'No command `brew`, https://brew.sh/');
	} else {
		throw new Error(`not support ${host_os} platform`);
	}

	var cmds, pkgCmd = false;

	if (typeof cmd == 'string') {
		cmds = [cmd];
	} else if (Array.isArray(cmd)) {
		cmds = cmd;
	} else if (typeof cmd == 'object') {
		cmds = cmd.cmds || [];
		for (var i in cmd.deps) {
			await install_check(i, cmd.deps[i]);
		}
		if (cmd.pkgCmds && cmd.pkgCmds.length) {
			pkgCmd = true;
			cmds = cmds.concat(cmd.pkgCmds);
		}
	}

	var cd = '';

	if (pkgCmd) {
		cd = `${__dirname}/../out/${app}`;
		// await exec2(`rm -rf ${cd}`);
		await exec2(`tar xfz ${__dirname}/pkgs/${app}.tar.gz -C ${__dirname}/../out/`);
	}

	for (var cmd of cmds) {
		if (cmd[0] == '*') {
			if (process.env.USER != 'root') {
				console.log(`Please input sudo password of execing ${cmd.substr(1)}`);
				cmd = 'sudo ' + cmd.substr(1);
			} else {
				cmd = cmd.substr(1);
			}
		}
		if (cd) {
			cmd = `cd ${cd} && ${cmd}`;
		}
		// process.stdin.setRawMode(true);
		process.stdin.resume();
		console.log('Install depe', app, 'Run:', cmd);

		await exec2(cmd);
	}
}

async function install_depe(opts, variables) {
	var {os,arch} = opts;
	var {cross_compiling} = variables;
	var dpkg = {};
	var pkgm = ''; // apt-get

	var pkgmErrMsg = cmd=>`Not found pkg install command "${cmd}" for the ${host_os}`;
	var pkgmCmds = (cmd)=>{
		if (pkgm == '') {
			if (host_os == 'linux') {
				if (execSync(`which apt-get`).code == 0) {
					pkgm = 'apt-get';
				} else if (execSync(`which yum`).code == 0) {
					pkgm = 'yum';
				} else if (execSync(`which apk`).code == 0) {
					pkgm = 'apk';
				} else {
					throw new Error(pkgmErrMsg('apt-get or yum'));
				}
			} else if (host_os == 'mac') {
				if (execSync(`which brew`).code == 0) {
					pkgm = 'brew';
				} else {
					throw new Error(pkgmErrMsg('brew'));
				}
			} else {
				throw new Error(`Not found pkg install command for the ${host_os}`);
			}
		}
		if (pkgm == 'apt-get') {
			return `*apt-get install ${cmd} -y`;
		} else if (pkgm == 'yum') {
			return `*yum install ${cmd} -y`;
		} else if (pkgm == 'apk') {
			return `*apk add ${cmd}`;
		} else if (pkgm == 'brew') {
			return `brew install ${cmd}`;
		}
	};

	// var cmake = { pkgCmds: [ `./configure`, `make -j2`, `*make -j1 install` ] };
	// var yasm = {
	// 	deps: {
	// 		autoconf: { pkgCmds: [ `./configure`, `make`, `*make install` ] },
	// 		automake: { pkgCmds: [ `./configure`, `make -j1`, `*make -j1 install` ] }
	// 	},
	// 	pkgCmds: [ './autogen.sh', 'make -j2', '*make install' ],
	// };
	var cmake = pkgmCmds('cmake');
	var yasm = pkgmCmds('yasm');
	dpkg.ninja = {
		deps: { cmake },
		pkgCmds: [ 'cmake .', 'make -j2', '*make install' ],
	};

	if (host_os == 'linux') {
		if (arch == 'x86' || arch == 'x64') {
			if (typeof yasm != 'string')
				yasm.deps.dtrace = pkgmCmds('systemtap-sdt-dev');
			dpkg.yasm = yasm;
		}
		if (os == 'linux') {
			if (cross_compiling) {
				if (arch == 'arm') {
					dpkg['arm-linux-gnueabihf-g++'] = pkgmCmds('g++-arm-linux-gnueabihf');
				} else if (arch == 'arm64') {
					dpkg['aarch64-linux-gnu-g++'] = pkgmCmds('g++-aarch64-linux-gnu');
				} else {
					throw new Error(`do not support cross compiling to "${arch}"`);
				}
			} else { // x86 or x64
				dpkg['g++'] = pkgmCmds('g++');
			}
			// TODO: Maybe also have to install libxcursor-dev and libfontconfig-dev
		} else if (os == 'android') {
			// dpkg.javac = pkgmCmds('default-jdk');
			dpkg.javac = pkgmCmds('openjdk-8-jdk');
		}
	}
	else if (host_os == 'mac') {
		if (arch == 'x86' || arch == 'x64') {
			dpkg.yasm = yasm;
		}
	}
	else {
		throw Error.new(`Cannot install compile depe for ${host_os} arch = ${arch}`);
	}

	for (var i in dpkg) {
		await install_check(i, dpkg[i]);
	}
}

function parseVersion(verStr) {
	if (verStr) {
		var mat = verStr.replace(/\.x/, '').match(/\d+(\.\d+)?/);
		if (mat) {
			return Number(mat[0]) || 0;
		}
	}
	return 0;
}

function get_host_tag_or_die() {
	// Return the host tag for this platform. Die if not supported.
	if (host_os == 'linux')
		return 'linux-x86_64';
	else if (host_os == 'mac')
		return 'darwin-x86_64';
	else if (host_os == 'win32' || host_os == 'cygwin')
		return 'windows-x86_64';
	throw Error('Unsupported platform: ' + host_os);
}

async function configure() {
	if (opts.help || opts.h) { // print help info
		console.log('');
		console.log('Usage: ./configure [VAR=VALUE]...');
		console.log('');
		console.log('Defaults for the options are specified in brackets.');
		console.log('');
		console.log('Configuration:');
		console.log('  ' + help_info.join('\n  '));
		return;
	}

	if ( ! fs.existsSync('out') ) {
		fs.mkdirSync('out');
	}

	//
	opts.arch = arch_format(opts.arch);
	var os = opts.os;
	var modile = (os == 'ios' || os == 'android');
	var arch = opts.arch;
	var suffix = arch;
	var configuration = opts.debug ? 'Debug': 'Release';
	var cross_compiling = arch != host_arch || host_os != os;
	var use_dtrace = is_use_dtrace();
	// Cross compiling is will have to snapshot?
	var v8_use_snapshot = /*!modile &&*/ !opts.without_snapshot;
	var emulator = 0;
	var OS = get_OS(os);
	var PYTHON = process.env.PYTHON || 'python';

	if ( isApple(os) ) {
		if ( opts.use_v8 == 'auto' ) { // all of mac use default javascriptcore
			opts.use_v8 = 0;
		}
	}
	var use_v8 = opts.use_v8 ? 1: isApple(os) ? 0: 1;

	var config_gypi = {
		target_defaults: {
			default_configuration: configuration,
		},
		variables: {
			configuration,
			asan: 0,
			host_node: process.execPath,
			host_os: host_os,    // v8 host_os
			host_arch: host_arch == 'x86' ? 'ia32' : host_arch,  // v8 host_arch
			target_arch: arch == 'x86' ? 'ia32' : arch,   // v8 target_arch
			arch: arch,
			arch_name: arch,
			suffix: suffix,
			debug: opts.debug,
			OS: OS,
			os: os,
			brand: '',
			clang: opts.clang,
			library: 'static_library',
			armv7: opts.armv7,
			armv7s: opts.armv7s,
			arm64: bi(arch == 'arm64'),
			arm_neon: opts.arm_neon,
			arm_vfp: opts.arm_vfp,
			arm_fpu: opts.arm_fpu,
			cross_compiling: bi(cross_compiling),
			media: opts.media,
			use_system_zlib: bi(os.match(/^(android|linux|ios|mac)$/)),
			use_gl: opts.use_gl ? 1: 0,
			use_v8: use_v8,
			use_openssl: bi(!opts.without_ssl),
			use_dtrace: bi(use_dtrace),
			use_js: opts.use_js,
			version_min: '',
			source: path.resolve(__dirname, '..'),
			output_name: '',
			output: '<(source)/<(output_name)',
			cc: 'gcc',
			cxx: 'g++',
			ld: 'g++',
			ar: 'ar',
			as: 'as',
			ranlib: 'ranlib',
			strip: 'strip',
			ld_host: 'g++',
			ar_host: 'ar',
			emulator: emulator,
			gcc_version: 0,
			android_api_level: android_api_level,
			build_sysroot: '/',
			build_bin: '/usr/bin',
			android_abi: '',
			ndk_path: '',
			xcode_version: 0,
			llvm_version: 0,
			gas_version: 0.0,
			byteorder: 'little',
			openssl_fips: '',
			openssl_no_asm: bi(os.match(/^(ios|android)$/) || arch.match(/arm/)),
			force_dynamic_crt: 0,
			v8_enable_gdbjit: 0,
			v8_no_strict_aliasing: 1,
			v8_optimized_debug: 0,
			v8_promise_internal_field_count: 1,
			v8_random_seed: 0,
			v8_trace_maps: 0,
			v8_use_snapshot: bi(v8_use_snapshot),
			v8_enable_i18n_support: 0,
			want_separate_host_toolset: bi(cross_compiling),
			python: PYTHON,
			V: opts.v,
		},
	};

	var variables = config_gypi.variables;

	if (opts.without_visibility_hidden) {
		variables.without_visibility_hidden = 1;
	}

	if (opts.without_embed_bitcode) {
		variables.without_embed_bitcode = 1;
	}

	// ----------------------- android/linux/ios/mac ----------------------- 

	if ( os == 'android' ) {
		// check android ndk toolchain
		var api = android_api_level;
		var ndk_path = opts.ndk_path || process.env.ANDROID_NDK;
		var toolchain_llvm = `${ndk_path}/toolchains/llvm/prebuilt/${get_host_tag_or_die()}`;
		var test = `${toolchain_llvm}/bin/armv7a-linux-androideabi${api}-clang`;
		// check ndk
		util.assert(fs.existsSync(test), `Don't found command of ${test}`);
		if (!fs.existsSync(`${__dirname}/ndk_llvm`)) {
			fs.symlinkSync(toolchain_llvm, `${__dirname}/ndk_llvm`);
		}
		if (!fs.existsSync(`${__dirname}/ndk`)) {
			fs.symlinkSync(ndk_path, `${__dirname}/ndk`);
		}
		var toolchain_dir = toolchain_llvm;
		// todo check android sdk ...

		await install_depe(opts, variables);

		var tools = {
			// 'mips': { cross_prefix: `mipsel-linux-android`, arch_name: 'mips2', abi: 'mips' },
			// 'mips64': { cross_prefix: `mips64el-linux-android`, arch_name: 'mips64r6', abi: 'mips64' },
			'arm': { cross_prefix: `arm-linux-androideabi-`, arch_name: 'armv6', abi: 'armeabi' },
			'arm64': { cross_prefix: `aarch64-linux-android-`, arch_name: 'armv8-a', abi: 'arm64-v8a' },
			'x86': { cross_prefix: `i686-linux-android-`, arch_name: 'i686', abi: 'x86' },
			'x64':  { cross_prefix: `x86_64-linux-android-`, arch_name: 'x86-64', abi: 'x86_64' },
		};

		var tool = tools[arch];
		if (!tool) {
			console.error(`do not support android os and ${arch} cpu architectures`);
			process.exit(-1);
		}

		var cross_prefix = tool.cross_prefix;
		var cc_prefix = tool.cross_prefix;

		if (arch == 'arm' && opts.armv7) {
			suffix = 'armv7';
			cc_prefix = 'armv7a-linux-androideabi-';
			tool.arch_name = 'armv7-a';
			tool.abi = 'armeabi-v7a';
		}

		variables.arch_name = tool.arch_name;
		variables.android_abi = tool.abi;
		variables.ndk_path = ndk_path;

		var cc_path = `${toolchain_dir}/bin/${cc_prefix}`;

		if (!fs.existsSync(`${cc_path}gcc`) || 
				!execSync(`${cc_path}gcc --version| grep -i gcc`).first || opts.clang
			) {
			if (!fs.existsSync(`${cc_path}clang`)) {
				cc_prefix = cc_prefix.replace(/-$/, '') + api + '-';
				cc_path = `${toolchain_dir}/bin/${cc_prefix}`;
				util.assert(fs.existsSync(`${cc_path}clang`), 
					`"gcc" or "clang" cross compilation was not found\n`);
			}
			if (!opts.clang) {
				// cannot find gcc compiler, use clang
				opts.clang = 1;
				variables.clang = 1; // use clang
				console.warn('\nOnly clang compiler can be used\n');
			}
		}

		if ( opts.clang ) { // use clang
			variables.cc = `${cc_prefix}clang`;
			variables.cxx = `${cc_prefix}clang++`;
			variables.ld = `${cc_prefix}clang++`;
		} else {
			variables.cc = `${cc_prefix}gcc`;
			variables.cxx = `${cc_prefix}g++`;
			variables.ld = `${cc_prefix}g++`;
		}

		if (host_os == 'mac') {
			variables.ld_host = 'android_LINK_host.mac'; // @tools
			variables.ar_host = 'android_AR_host.mac';
		}

		if (!fs.existsSync(`${toolchain_dir}/bin/${cross_prefix}as`)) {
			cross_prefix = fs.existsSync(`${toolchain_dir}/bin/llvm-as`) ? 'llvm-': '';
		}
		variables.as = `${cross_prefix}as`;
		variables.ar = `${cross_prefix}ar`;
		variables.ranlib = `${cross_prefix}ranlib`;
		variables.strip = `${cross_prefix}strip`;
		variables.build_bin = `${toolchain_dir}/bin`;
		variables.build_sysroot = `${toolchain_dir}/sysroot`;

		if (opts.clang) { // llvm clang
			var llvm_version = execSync(`${cc_path}clang \
				--version`).first.match(/LLVM (\d+\.\d+(\.\d+)?)/i);
			variables.llvm_version = llvm_version && Number(llvm_version[1]) || 0;
		} else {
			var gcc_version = execSync(`${cc_path}gcc \
				--version| grep -i gcc | awk '{ print $3 }'`).first;
			variables.gcc_version = parseVersion(gcc_version);
		}
	}
	else if ( os == 'linux' ) {

		if ( ['arm', 'arm64', 'x86', 'x64'].indexOf(arch) == -1 ) {
			console.error(`do not support linux os and ${arch} cpu architectures`);
			return;
		}
		if ( host_os != 'linux' ) {
			console.error(`You can compile targets ${arch} only on Linux systems.`);
			return;
		}
		if (host_arch != 'x86' && host_arch != 'x64' && (arch == 'x86' || arch == 'x64')) {
			console.error(`You can compile targets ${arch} only on X86 or x64 machine Linux systems.`);
			return;
		}
		if ( opts.clang ) {
			variables.clang = 0;
			console.warn('The Linux system calls the clang compiler to use GCC.');
		}

		var [uname] = syscall('uname -a').stdout;
		if (/ubuntu/i.test(uname)) {
			variables.brand = 'ubuntu';
		} else if (/debian/i.test(uname)) {
			variables.brand = 'debian';
		} else {
			console.warn('Unknown Linux distribution');
		}

		await install_depe(opts, variables);

		if ( arch == 'arm' || arch == 'arm64' ) { // arm arm64
			if (arch == 'arm') {
				if (opts.armv7) {
					suffix = 'armv7';
					variables.arch_name = 'armv7-a';
				} else {
					suffix = 'armv6';
					variables.arch_name = 'armv6';
				}
			} else if (arch == 'arm64') {
				variables.arch_name = 'armv8-a';
			}
		} else { // x86 x64
			variables.arch_name = arch == 'x86' ? 'i386' : 'x86-64';
		}

		// check compiler and set sysroot
		if ( (host_arch == 'x86' || host_arch == 'x64') && (arch == 'arm' || arch == 'arm64') ) {

			['gcc', 'g++', 'g++', 'ar', 'as', 'ranlib', 'strip'].forEach((e,i)=>{
				var r;
				if (arch == 'arm64') {
					r = syscall(`find /usr/bin -name aarch64-linux-gnu*${e}*`)
						.stdout.sort((a,b)=>a.length - b.length)[1];
				}
				if (!r) {
					r = syscall(`find /usr/bin -name arm-linux-gnueabihf*${e}*`)
						.stdout.sort((a,b)=>a.length - b.length)[1];
				}
				if (!r) {
					r = syscall(`find /usr/bin -name arm-linux*${e}*`)
						.stdout.sort((a,b)=>a.length - b.length)[1];
				}
				util.assert(r, `"arm-linux-${e}" cross compilation was not found\n`);
				variables[['cc', 'cxx', 'ld', 'ar', 'as', 'ranlib', 'strip'][i]] = r;
			});

		} else {
			['gcc', 'g++', 'ar', 'as', 'ranlib', 'strip'].forEach(e=>{
				util.assert(!execSync('which ' + e).code, `${e} compile command was not found`);
			});
		}

		// gcc version
		var gcc_version = execSync(`${variables.cc} \
			--version| grep gcc | awk '{ print $4 }'`).first;
		variables.gcc_version = parseVersion(gcc_version);
	}
	else if (os == 'ios' || os == 'mac') {
		if ( os == 'ios' ) {
			if ( host_os != 'mac' ) {
				console.error(
					'Only in the mac os and the installation of the \
					Xcode environment to compile target iOS');
				return;
			}
			if ( ['arm', 'arm64', 'x86', 'x64'].indexOf(arch) == -1) {
				// console.error(`do not support iOS and ${arch} cpu architectures`);
				return;
			}
		} else {
			if ( ['x86', 'x64', 'arm64'].indexOf(arch) == -1) {
				console.error(`do not support MacOSX and ${arch} cpu architectures`);
				return;
			}
			console.warn();
		}

		var XCODEDIR = syscall('xcode-select --print-path').first;

		util.assert(XCODEDIR, 'The Xode installation directory could not be found. '+
			'Make sure that Xcode is installed correctly');

		await install_depe(opts, variables);

		try {
			variables.xcode_version = parseVersion(syscall('xcodebuild -version').first);
			variables.llvm_version  = parseVersion(syscall('cc --version').first);
		} catch(e) {}

		if ( arch == 'arm' ) {
			suffix = opts.armv7s ? 'armv7s' : 'armv7';
			variables.arch_name = opts.armv7s ? 'armv7s' : 'armv7';
		} else if ( arch == 'x86' ) {
			variables.arch_name = 'i386';
		} else if ( arch == 'x64' ) {
			variables.arch_name = 'x86_64';
		}

		if ( os == 'ios' ) {
			if (arch == 'x86' || arch == 'x64' || opts.emulator) {
				variables.emulator = 1;
				console.log('enable emulator');
				variables.build_sysroot = syscall('xcrun --sdk iphonesimulator --show-sdk-path').first;
			} else {
				variables.build_sysroot = syscall('xcrun --sdk iphoneos --show-sdk-path').first;
			}
			variables.version_min = '10.0';
		} else { // mac
			variables.build_sysroot = `${XCODEDIR}/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk`;
			variables.version_min = '10.15';
		}

		variables.clang = 1;
		variables.cc = 'clang';
		variables.cxx = 'clang';
		variables.ld = 'clang++';
		variables.ld_host = 'clang++';
		variables.build_bin = `${XCODEDIR}/Toolchains/XcodeDefault.xctoolchain/usr/bin`;
	}
	else {
		console.error(`do not support ${os} os`);
		return;
	}

	if (variables.emulator) {
		suffix += '.emulator';
	}
	if (opts.suffix) {
		suffix = `${opts.suffix}.${suffix}`;
	}

	var brand = variables.brand;
	var output = `${os}${brand&&'-'+brand}.${suffix}.${configuration}`;

	variables.output_name = output;
	variables.output = path.resolve(`${__dirname}/../out/${output}`);
	variables.suffix = suffix;

	var config_mk = [
		'# Do not edit. Generated by the configure script.',
		`OS=${os}`,
		`ARCH=${arch}`,
		`BUILDTYPE=${configuration}`,
		`V=${opts.v}`,
		`SUFFIX=${suffix}`,
		`OUTPUT=${output}`,
		`ANDROID_API_LEVEL=${android_api_level}`,
		`export CC_target:=${variables.cc}`,
		`export CXX_target:=${variables.cxx}`,
		`export LINK_target:=${variables.ld}`,
		`export AR_target:=${variables.ar}`,
		`export LINK_host:=${variables.ld_host}`,
		`export AR_host:=${variables.ar_host}`,
		`export AS:=${variables.as}`,
		`export STRIP:=${variables.strip}`,
		`export RANLIB:=${variables.ranlib}`,
		`export SYSROOT:=${variables.build_sysroot}`,
		`export PATH:=${__dirname}:${variables.build_bin}:$(PATH)`,
		`export PYTHON:=${PYTHON}`,
	];

	var java_home = process.env.JAVA7_HOME || process.env.JAVA_HOME;
	if ( java_home ) {
		config_mk.push(`export JAVAC:=${java_home}/bin/javac`);
		config_mk.push(`export JAR:=${java_home}/bin/jar`);
	}

	{ // configure ffmpeg
		var ff_install_dir = `${variables.output}/obj.target/ffmpeg`;
		var ff_rebuild = false;
		var ff_product_path = `${ff_install_dir}/libffmpeg.a`;

		if ( opts.media == 'auto' ) { // auto
			if ( !fs.existsSync(`${ff_product_path}`) &&
					 !fs.existsSync(`${ff_install_dir}/objs`) ) {
				ff_rebuild = true;
			} else {
				ff_rebuild = !fs.existsSync(__dirname + '/../deps/ffmpeg/config.h')
			}
			variables.media = 1;
		} else {
			if ( opts.media ) { // Force rebuild ffmpeg
				ff_rebuild = true;
			}
		}

		if ( ff_rebuild ) { // rebuild ffmpeg
		 if ( !configure_ffmpeg(opts, config_gypi, opts.clang, ff_install_dir) ) {
			 return;
		 }
		}
	}

	// ------------------ output config.mk, config.gypi ------------------ 
	
	var config_gypi_str = JSON.stringify(config_gypi, null, 2);
	var config_mk_str = config_mk.join('\n') + '\n';

	{
		console.log('\nConfiguration output:');
		for (var i in opts) {
			console.log(' ', i, ':', opts[i]);
		}
		console.log('');
		console.log(config_gypi_str, '\n');
		console.log(config_mk_str);
	}

	fs.writeFileSync('out/config.gypi', config_gypi_str);
	fs.writeFileSync('out/config.mk', config_mk_str);

	touch_files(variables);
}

configure().then(e=>{
	process.exit(0);
}).catch(e=>{
	console.error(e);
	process.exit(1);
});
