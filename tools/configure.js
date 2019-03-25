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

var util = require('../libs/qgr-utils');
var fs = require('../libs/qgr-utils/fs');
var url = require('../libs/qgr-utils/url');
var path = require('path');
var host_os = process.platform == 'darwin' ? 'osx': process.platform;
var host_arch = arch_format(process.arch);
var argument = require('../libs/qgr-utils/arguments');
var { syscall, execSync } = require('../libs/qgr-utils/syscall');
var opts = argument.options;
var help_info = argument.helpInfo;
var def_opts = argument.defOpts;
var default_arch = host_arch || 'x86';
var android_api_level = 21;

def_opts(['help','h'], 0,       '-h, --help     print help info');
def_opts('v', 0,                '-v, --v        enable compile print info [{0}]');
def_opts('debug', 0,            '--debug        enable debug status [{0}]');
def_opts('os', host_os,         '--os=OS        system type ios/android/osx/linux/win [{0}]');
def_opts('arch', default_arch,  '--arch=CPU     cpu type options arm/arm64/mips/mips64/x86/x64 [{0}]');
def_opts('library', 'static',   '--library=LIB  compile output library type static/shared [{0}]');
def_opts('armv7', arm(),        '--armv7        enable armv7 [{0}]');
def_opts('armv7s', 0,           '--armv7s       enable armv7s form apple iphone [{0}]');
def_opts('arm-neon', arm(),     '--arm-neon     enable arm neno [{0}]');
def_opts('arm-vfp', opts.arch == 'arm64' ? 'vfpv4': 
										(opts.arch == 'arm' ? (opts.armv7 || opts.armv7s ? 'vfpv3' : 'vfpv2'): 'none'),
																'--arm-vfp=VAL  enable arm vfp options vfpv2/vfpv3/vfpv4/none [{0}]');
def_opts('arm-fpu', opts.arm_neon ? 'neon': opts.arm_vfp, 
																'--arm-fpu=VAL  enable arm fpu [{0}]');
def_opts('clang', opts.os.match(/osx|ios|iwatch|tvos/) ? 1 : 0, 
																'--clang        enable clang compiler [{0}]');
def_opts('media', 'auto',       '--media        compile media [{0}]');
def_opts(['ndk-path','ndk'], '','--ndk-path     android NDK path [{0}]');
def_opts(['use-v8','v8'],'auto','--use-v8,-v8   force use javascript v8 library [{0}]');
def_opts('without-snapshot', 0, '--without-snapshot without snapshot for v8 [{0}]');
def_opts('without-ssl', 0,      '--without-ssl  build without SSL (disables crypto, https, inspector, etc.)');
def_opts('with-intl', 0,        '--with-intl    enable intl default:[{0}]');
def_opts('prefix', '/usr/local','--prefix       select the install prefix [{0}]');
def_opts('no-browser-globals', 0,'--no-browser-globals do not export browser globals like setTimeout, console, etc. [{0}]' +
																 '(This mode is not officially supported for regular applications)');
def_opts('without-inspector', 0,'--without-inspector disable the V8 inspector protocol [{0}]');
def_opts('without-visibility-hidden', 0, 
																'--without-visibility-hidden without visibility hidden [{0}]');
def_opts('suffix', '',          '--suffix=VAL Compile directory suffix [{0}]');
def_opts('without-embed-bitcode', 1,
																'--without-embed-bitcode disable apple embed-bitcode [{0}]');
def_opts('without-node', 0,     '--without-node disable node [{0}]');
def_opts('more-log',     0,     '--more-log print more log message [{0}]');

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
			fs.writeFileSync(pathname, '');
		}
	});
}

function configure_FFmpeg(opts, variables, configuration, clang, ff_install_dir) {
	var os = opts.os;
	var arch = opts.arch;
	var cmd = '';
	var source = __dirname + '/../depe/FFmpeg';

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
		'--disable-swscale',
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
		// enable protocols
		'--enable-protocol=hls',
		'--enable-protocol=http',
		'--enable-protocol=file',
		'--enable-protocol=rtmp',
	];

	if (os == 'android') {
		cmd = `\
			./configure \
			--target-os=android \
			--arch=${arch} \
			--sysroot=${variables.build_sysroot} \
			--cross-prefix=${variables.cross_prefix} \
			--enable-cross-compile \
		`;
			// --enable-jni \
			// --enable-mediacodec \

		var cc = variables.cc;
		var cflags = '-ffunction-sections -fdata-sections ';

		if ( !clang ) { // use gcc 
			cflags += '-funswitch-loops ';
		}

		cmd += `--cc='${cc} ${cflags} -march=${variables.arch_name}' `;
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
			cmd += `--cc='${cc} ${cflags} -march=${variables.arch_name}' `;
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
		// apple marker
		// -fembed-bitcode-marker
		var f_embed_bitcode = opts.without_embed_bitcode ?  '' : '-fembed-bitcode';

		cmd += `--cc='clang -miphoneos-version-min=10.0 -arch ${variables.arch_name} ${f_embed_bitcode}' `; 
		if (arch == 'x86' || arch == 'x64') {
			cmd += '--sysroot=$(xcrun --sdk iphonesimulator --show-sdk-path) ';
		} else {
			cmd += '--sysroot=$(xcrun --sdk iphoneos --show-sdk-path) ';
		}
	} 
	else if (os == 'osx') {
		cmd = `\
			./configure \
			--target-os=darwin \
			--arch=${arch} \
		`;
		cmd += `--cc='clang -mmacosx-version-min=10.7 -arch ${variables.arch_name} ' `;
		cmd += '--sysroot=$(xcrun --sdk macosx --show-sdk-path) ';
	}
	
	if ( !cmd ) {
		return false;
	}
	
	if ( opts.library == 'shared' ) {
		ff_opts.push('--enable-pic');
	}
	if ( opts.debug ) {
		ff_opts.push('--enable-debug=2');
	} else {
		ff_opts.push('--disable-logging');
		ff_opts.push('--disable-debug'); 
		ff_opts.push('--strip');
		// ff_opts.push('--optflags="-O3"');
	}

	cmd += ff_opts.join(' ');

	// clean
	execSync(`cd depe/FFmpeg; make clean; find . -name *.o|xargs rm; `);
	syscall(`
		rm -rf ${variables.output}/obj.target/depe/FFmpeg/*;
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
	`);

	console.log('FFMpeg Configuration:\n');
	console.log(`export PATH=${__dirname}:${variables.build_bin}:$PATH`);
	console.log(cmd, '\n');

	var log = syscall(
		`export PATH=${__dirname}:${variables.build_bin}:$PATH; cd depe/FFmpeg; ${cmd};`
	);
	console.error(log.stderr.join('\n'));
	console.log(log.stdout.join('\n'));

	return true;
}

function bs(a) {
	return a ? 'true' : 'false';
}

function bi(a) {
	return a ? 1 : 0;
}

function GetFlavor(params = {}) {
	// """Returns |params.flavor| if it's set, the system's default flavor else."""
	var flavors = {
		'cygwin': 'win',
		'win32': 'win',
		'darwin': 'osx',
		'osx': 'osx',
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
	return ['solaris', 'osx', 'linux', 'freebsd'].indexOf(flavor) > -1;
}

function get_OS(os) {
	var OS = ['ios', 'osx', 'tvos', 'iwatch'].indexOf(os) > -1 ? 'mac' : os
	return OS;
}

function configure_node(opts, variables, configuration) {
	
	var cfg = {
		coverage: 'false',
		debug_devtools: 'node',
		debug_http2: 'false',
		debug_nghttp2: 'false',
		icu_small: 'false',
		node_enable_d8: 'false',
		node_enable_v8_vtunejit: 'false',
		node_install_npm: 'false',
		node_module_version: 57,
		node_no_browser_globals: bs(opts.no_browser_globals),
		node_prefix: opts.prefix,
		node_release_urlbase: '',
		node_shared: 'false',
		node_shared_cares: 'false',
		node_shared_http_parser: 'true',
		node_shared_libuv: 'true',
		node_shared_openssl: 'true',
		node_shared_zlib: 'true',
		node_tag: '',
		node_use_etw: 'false',
		node_use_lttng: 'false',
		node_use_perfctr: 'false',
		node_use_v8_platform: bs(opts.use_v8),
		node_use_bundled_v8: 'false',
		node_without_node_options: 'false',
		shlib_suffix: '',
		v8_enable_i18n_support: bi(opts.use_v8 && opts.with_intl),
	};

	Object.assign(variables, cfg);

	if ( variables.v8_enable_i18n_support ) {
		// configure node 
		syscall(`cd ${__dirname}/../node; ./configure`);

		var config = fs.readFileSync(__dirname + '/../node/icu_config.gypi', 'utf8');
		config = config.replace(/'[^']+derb\.c(pp)?',?/g, '');
		fs.writeFileSync(__dirname + '/../node/icu_config.gypi', config);
		config = fs.readFileSync(__dirname + '/../node/config.gypi', 'utf8');
		config = config.replace(/^#.+/gm, '');
		config = eval('(' + config + ')').variables;
		fs.rm_r_sync(__dirname + '/../node/out');
		fs.rm_r_sync(__dirname + '/../node/config.mk');

		variables.icu_ver_major = config.icu_ver_major;
		variables.icu_data_file = config.icu_data_file;
		variables.icu_data_in = config.icu_data_in;
		variables.icu_endianness = config.icu_endianness;
		variables.icu_locales = config.icu_locales;
		variables.icu_gyp_path = 
			url.relative(__dirname + '/../out', `${__dirname}/../node/${config.icu_gyp_path}`);
		variables.icu_path = 
			url.relative(__dirname + '/../out', `${__dirname}/../node/${config.icu_path}`);
		variables.icu_small = 'true';
	}
}

function configure() {

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
	// 
	opts.arch = arch_format(opts.arch);
	var os = opts.os;
	var modile = (os == 'ios' || os == 'android');
	var arch = opts.arch;
	var suffix = arch;
	var configuration = opts.debug ? 'Debug': 'Release';
	var cross_compiling = arch != host_arch;
	var use_dtrace = is_use_dtrace();
	/* 交叉编译时需要单独的工具集来生成v8-js快照,所以交叉编译暂时不使用v8-js快照*/
	var v8_use_snapshot = !opts.without_snapshot && !cross_compiling && !modile;
	var shared = opts.library == 'shared' ? 'shared': '';

	if ( os == 'ios' ) {
		if ( opts.use_v8 == 'auto' ) { // ios默认使用 javascriptcore
			if ( arch != 'x86' && arch != 'x64' ) {
				opts.use_v8 = 0;
			}
		}
	}
	opts.use_v8 = bi(opts.use_v8);

	var config_gypi = {
		target_defaults: {
			default_configuration: configuration,
		},
		variables: {
			/* config */
			asan: 0,
			host_node: process.execPath,
			host_os: host_os == 'osx' ? 'mac' : host_os,    // v8 host_os
			host_arch: host_arch == 'x86' ? 'ia32' : host_arch,  // v8 host_arch
			target_arch: arch == 'x86' ? 'ia32' : arch,   // v8 target_arch
			arch: arch,
			arch_name: arch,
			suffix: suffix,
			debug: opts.debug,
			OS: get_OS(opts.os),
			os: opts.os,
			use_v8: opts.use_v8,
			more_log: opts.more_log,
			clang: opts.clang,
			library: 'static_library',
			library_output: opts.library + '_library',
			armv7: opts.armv7,
			armv7s: opts.armv7s,
			arm64: bi(arch == 'arm64'),
			arm_neon: opts.arm_neon,
			arm_vfp: opts.arm_vfp,
			arm_fpu: opts.arm_fpu,
			cross_compiling: bi(cross_compiling),
			cross_prefix: '',
			use_system_zlib: bi(os.match(/^(android|linux|ios|osx)$/)),
			media: opts.media,
			version_min: '',
			output: '',
			cc: 'gcc',
			cxx: 'g++',
			ld: 'g++',
			ar: 'ar',
			as: 'as',
			gcc_version: 0,
			ranlib: 'ranlib',
			strip: 'strip',
			build_sysroot: '/',
			build_bin: '/usr/bin',
			build_tools: __dirname,
			android_abi: '',
			xcode_version: 0,
			llvm_version: 0,
			// depes config
			openssl_fips: '',
			openssl_no_asm: bi(os.match(/^(ios|android)$/)),
			node_enable: 0,
			node_byteorder: 'little',
			node_use_openssl: bs(!opts.without_ssl),
			node_use_dtrace: bs(use_dtrace),
			uv_use_dtrace: bs(use_dtrace),
			force_dynamic_crt: 0,
			v8_enable_gdbjit: 0,
			v8_enable_inspector: bi(opts.use_v8 && !opts.without_inspector && !opts.without_ssl),
			v8_no_strict_aliasing: 1,
			v8_optimized_debug: 0,
			v8_promise_internal_field_count: 1,
			v8_random_seed: 0,
			v8_trace_maps: 0,
			v8_use_snapshot: bs(v8_use_snapshot),
			v8_enable_i18n_support: 0,
			want_separate_host_toolset: bi(v8_use_snapshot &&cross_compiling && !opts.without_snapshot),
			want_separate_host_toolset_mkpeephole: bi(v8_use_snapshot && cross_compiling),
		},
	};
	
	var variables = config_gypi.variables;

	if (opts.without_visibility_hidden) {
		variables.without_visibility_hidden = 1;
	}

	if (opts.without_embed_bitcode) {
		variables.without_embed_bitcode = 1;
	}

	if ( use_dtrace ) {
		variables.uv_parent_path = '/deps/uv/';
	}

	configure_node(opts, variables, configuration);

	var config_mk = [
		'# Do not edit. Generated by the configure script.',
		'OS=' + os,
		'ARCH=' + arch,
		'BUILDTYPE=' + configuration,
		'V=' + opts.v,
	];
	var ENV = [];

	// ----------------------- android/linux/ios/osx ----------------------- 

	if ( os == 'android' ) {
		var api = android_api_level;

		// check android toolchain
		var toolchain_dir = `${__dirname}/android-toolchain/${arch}`;
		if (!fs.existsSync(toolchain_dir)) {
			var toolchain_dir2 = `${__dirname}/android-toolchain/arm`;
			// chech ndk r19
			if ( fs.existsSync(`${toolchain_dir2}/bin/armv7a-linux-androideabi${api}-clang`) ) {
				opts.clang = 1; // use clang
				variables.clang = 1;
				toolchain_dir = toolchain_dir2;
			} else {
				var ndk_path = opts.ndk_path || `${process.env.ANDROID_HOME}/ndk-bundle`;
				if ( ndk_path && fs.existsSync(ndk_path) ) { // install tool
					syscall(`${__dirname}/install-android-toolchain ${ndk_path} ${api} ${arch}`);
				} else {
					console.error(
						`Please run "./tools/install-android-toolchain NDK-DIR" ` +
						'to install android toolchain!');
					process.exit(1);
				}
			}
		}

		var tools = {
			'arm': { cross_prefix: `arm-linux-androideabi-`, arch_name: 'armv6', abi: 'armeabi' },
			'arm64': { cross_prefix: `aarch64-linux-android-`, arch_name: 'armv8-a', abi: 'arm64-v8a' },
			// 'mips': { cross_prefix: `mipsel-linux-android`, arch_name: 'mips2', abi: 'mips' },
			// 'mips64': { cross_prefix: `mips64el-linux-android`, arch_name: 'mips64r6', abi: 'mips64' },
			'x86': { cross_prefix: `i686-linux-android-`, arch_name: 'i686', abi: 'x86' },
			'x64':  { cross_prefix: `x86_64-linux-android-`, arch_name: 'x86-64', abi: 'x86_64' },
		};

		var tool = tools[arch];
		if (!tool) {
			console.error(`do not support android os and ${arch} cpu architectures`);
			return;
		}

		if (arch == 'arm' && opts.armv7) {
			suffix = 'armv7';
			tool.arch_name = 'armv7-a';
			tool.abi = 'armeabi-v7a';
		}

		var cc_prefix = tool.cross_prefix;
		var cc_path = `${toolchain_dir}/bin/${cc_prefix}`;

		if (fs.existsSync(`${cc_path.replace(/-$/, '')}${api}-clang`)) {
			cc_prefix = cc_prefix.replace(/-$/, '') + api + '-';
			cc_path = `${toolchain_dir}/bin/${cc_prefix}`;
		}

		variables.cross_prefix = tool.cross_prefix;
		variables.arch_name = tool.arch_name;
		variables.android_abi = tool.abi;

		if (!fs.existsSync(`${cc_path}gcc`) || 
				!execSync(`${cc_path}gcc --version| grep -i gcc`).stdout[0]
			) {
			// cannot find gcc compiler, use clang
			opts.clang = 1;
			variables.clang = 1; // use clang
		}

		if ( opts.clang ) { // use clang
			util.assert(fs.existsSync(`${cc_path}clang`), 
				`"${cc_prefix}clang" or "${cc_prefix}gcc" cross compilation was not found\n`);
			variables.cc = `${cc_prefix}clang`;
			variables.cxx = `${cc_prefix}clang++`;
			variables.ld = `${cc_prefix}clang++`;
		} else {
			variables.cc = `${cc_prefix}gcc`;
			variables.cxx = `${cc_prefix}g++`;
			variables.ld = `${cc_prefix}g++`;

			if (fs.existsSync(`${cc_path}clang++`)) {
				/* 
				 * 这里使用g++进行链接会导致无法运行,
				 * 这可能是g++默认链接的stl库有问题.不再追究更多细节,使用clang++进行链接
				 */
				variables.ld = `${cc_prefix}clang++`;
			}
		}
		variables.ar = `${variables.cross_prefix}ar`;
		variables.as = `${variables.cross_prefix}as`;
		variables.ranlib = `${variables.cross_prefix}ranlib`;
		variables.strip = `${variables.cross_prefix}strip`;
		variables.build_bin = `${toolchain_dir}/bin`;
		variables.build_sysroot = `${toolchain_dir}/sysroot`;

		if (opts.clang) {
			var llvm_version = execSync(`${cc_path}clang \
				--version`).stdout[0].match(/LLVM (\d+\.\d+(\.\d+)?)/i);
			variables.llvm_version = llvm_version && llvm_version[1] || 0;
		} else {
			var gcc_version = execSync(`${cc_path}gcc \
				--version| grep -i gcc | awk '{ print $3 }'`).stdout[0];
			variables.gcc_version = gcc_version ? gcc_version.replace(/\.x/, '') : 0;
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
			console.warn('The Linux system calls the clang compiler to use GCC.');
		}

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
				util.assert(!execSync('which ' + e).code, `${e} command was not found`);
			});
		}

		// gcc version
		var gcc_version = execSync(`${variables.cc} \
			--version| grep gcc | awk '{ print $4 }'`).stdout[0];
		variables.gcc_version = gcc_version || 0;
	}
	else if (os == 'ios' || os == 'osx') {

		if ( os == 'ios' ) {
			if ( host_os != 'osx' ) {
				console.error(
					'Only in the osx os and the installation of the \
					Xcode environment to compile target iOS');
				return;
			}
			if ( ['arm', 'arm64', 'x86', 'x64'].indexOf(arch) == -1) {
				console.error(`do not support iOS and ${arch} cpu architectures`);
				return;
			}
		} else {
			if ( ['x86', 'x64'].indexOf(arch) == -1) {
				console.error(`do not support MacOSX and ${arch} cpu architectures`);
				return;
			}
			console.warn();
			console.warn('******************** Unrealized MacOSX Protform ********************');
		}

		var XCODEDIR = syscall('xcode-select --print-path').stdout[0];

		try {
			variables.xcode_version = syscall('xcodebuild -version').stdout[0].match(/\d+.\d+$/)[0];
			variables.llvm_version = 
				syscall('cc --version').stdout[0].match(/clang-(\d+\.\d+(\.\d+)?)/i)[1];
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
			if (arch == 'x86' || arch == 'x64') {
				variables.build_sysroot = syscall('xcrun --sdk iphonesimulator --show-sdk-path').stdout[0];
			} else {
				variables.build_sysroot = syscall('xcrun --sdk iphoneos --show-sdk-path').stdout[0];
			}
			variables.version_min = '10.0';
		} else { // osx
			variables.build_sysroot = `${XCODEDIR}/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk`;
			variables.version_min = '10.7';
		}
		
		variables.cc = 'clang';
		variables.cxx = 'clang';
		variables.ld = 'clang++';
		variables.ar = 'ar'; 
		variables.as = 'as';
		variables.ranlib = 'ranlib';
		variables.strip = 'strip';
		variables.build_bin = `${XCODEDIR}/Toolchains/XcodeDefault.xctoolchain/usr/bin`;
	}
	else {
		console.error(`do not support ${os} os`);
		return;
	}

	if (opts.suffix) {
		suffix = String(opts.suffix);
	}

	variables.output = path.resolve(`${__dirname}/../out/${os}.${suffix}.${configuration}`);
	variables.suffix = suffix;

	if (shared) 
		variables.output += '.' + shared;

	config_mk.push('SUFFIX=' + suffix);
	config_mk.push('CXX=' + variables.cxx);
	config_mk.push('LINK=' + variables.ld);
	config_mk.push('SHARED=' + shared);

	ENV.push('export CC=' + variables.cc);
	ENV.push('export CXX=' + variables.cxx);
	ENV.push('export LINK=' + variables.ld);
	ENV.push('export AR=' + variables.ar);
	ENV.push('export AS=' + variables.as);
	ENV.push(`export PATH=${__dirname}:${variables.build_bin}:$$PATH`);
	ENV.push(`export SYSROOT=${variables.build_sysroot}`);

	var java_home = process.env.JAVA7_HOME || process.env.JAVA_HOME;
	if ( java_home ) {
		config_mk.push(`JAVAC=${java_home}/bin/javac`);
		config_mk.push(`JAR=${java_home}/bin/jar`);
	}
	
	// -------------------------- configure FFmpeg --------------------------

	{ // configure FFmpeg
		var ff_install_dir = `${variables.output}/obj.target/FFmpeg`;
		var ff_rebuild = false;
		var ff_product_path = `${ff_install_dir}/libFFmpeg.a`;

		if ( opts.media == 'auto' ) { // auto
			if ( !fs.existsSync(`${ff_product_path}`) &&
					 !fs.existsSync(`${ff_install_dir}/objs`) ) {
				ff_rebuild = true;
			}
			variables.media = 1;
		} else {
			if ( opts.media ) { // Force rebuild FFmpeg
				ff_rebuild = true;
			}
		}

		if ( ff_rebuild ) { // rebuild FFmpeg
		 if ( !configure_FFmpeg(opts, variables, configuration, opts.clang, ff_install_dir) ) {
			 return;
		 }
		}
	}

	// ------------------ output config.mk, config.gypi ------------------ 
	
	var config_gypi_str = JSON.stringify(config_gypi, null, 2);
	var config_mk_str = config_mk.join('\n') + '\n';
	config_mk_str += 'ENV=\\\n';
	config_mk_str += ENV.join(';\\\n') + ';\n';

	{
		console.log('\nConfiguration output:');
		for (var i in opts) {
			console.log(' ', i, ':', opts[i]);
		}
		console.log('');
		console.log(config_gypi_str, '\n');
		console.log(config_mk_str);
	}

	if ( ! fs.existsSync('out') ) {
		fs.mkdirSync('out');
	}

	touch_file([ 
		'out/native-inl-js.cc',
		'out/native-ext-js.cc',
		'out/native-lib-js.cc',
		'out/native-font.cc',
		'out/native-glsl.cc',
	]);
	
	fs.writeFileSync('out/config.gypi', config_gypi_str);
	fs.writeFileSync('out/config.mk', config_mk_str);

	if (!opts.without_node) { 
		fs.writeFileSync('depe/node/config.gypi', '\n' + config_gypi_str.replace(/"([^"]*)"/g, "'$1'"));
	}
}

configure();
