{ 
	'variables': {
		'without_visibility_hidden%': 0,
	},
	'targets': [
		{
			'target_name': 'test1',
			'type': 'executable',
			'include_dirs': [
				'../out',
			],
			'dependencies': [
				'flare-util',
				'skia',
				'flare',
				'flare-media',
				# 'flare-js',
				# 'flare-node',
				###########
				'trial',
				'deps/ffmpeg/ffmpeg.gyp:ffmpeg',
				'deps/freetype2/freetype2.gyp:ft2',
			],
			'mac_bundle': 1,
			'mac_bundle_resources': [
				'res',
				'test-flare',
				'../examples',
				'../bench',
			],
			'xcode_settings': {
				'OTHER_LDFLAGS': ['-all_load'],
			},
			'sources': [
				'../examples',
				'../libs/fproj',
				'../libs/somes',
				'test.cc',
				'test-fs.cc',
				'test-fs2.cc',
				# 'test-freetype.cc',
				'test-json.cc',
				'test-string.cc',
				'test-list.cc',
				'test-map.cc',
				'test-event.cc',
				'test-zlib.cc',
				'test-http.cc',
				'test-http2.cc',
				'test-http3.cc',
				'test-https.cc',
				'test-thread.cc',
				'test-ffmpeg.cc',
				'test-number.cc',
				'test-uv.cc',
				'test-net.cc',
				'test-fs-async.cc',
				'test-ssl.cc',
				'test-net-ssl.cc',
				'test-http-cookie.cc',
				'test-localstorage.cc',
				'test-buffer.cc',
				'test-jsc.cc',
				'test-loop.cc',
				'test-os.cc',
				'test-mutex.cc',
				'test-ios-run-loop.cc', 
				# 'test-benchmark.cc',
				# 'test-demo.cc',
				# 'test-v8.cc',
				# 'test-gui.cc',
				'test-sizeof.cc',
				'test-util.cc',
				'test-alsa-ff.cc',
				'test-linux-input.cc',
				'test-linux-input-2.cc',
				'test-jsx.cc',
			],
			'conditions': [
				['use_skia==1', {
					'sources': [
						'test-skia.cc',
						'test-scaner.cc',
						'test-layout.cc',
					],
				}],
				['os in "ios osx"', {
					'sources': [
						'test-<(os).plist',
						'Storyboard-<(os).storyboard',
					],
					'xcode_settings': {
						# 'INFOPLIST_FILE': '$(SRCROOT)/test/test-<(os).plist',
						'INFOPLIST_FILE': '<(output)/../../test/test-<(os).plist',
					},
					'link_settings': {
						'libraries': [
							'$(SDKROOT)/System/Library/Frameworks/JavaScriptCore.framework',
						],
					},
				}],
				['os in "linux android" and library_output=="static_library"', {
					'ldflags': [ '<@(other_ldflags)' ],
				}],
			],
		},
	],

	'conditions': [
		# gen android test depes `libflare-depes-test.so`
		['os=="android" and (debug==1 or without_visibility_hidden==1)', {
			'targets': [
			{
				'target_name': 'flare-depes-test',
				'type': 'shared_library',
				'dependencies': [
					'flare/util/minizip.gyp:minizip',
					'deps/tess2/tess2.gyp:tess2', 
					'deps/freetype2/freetype2.gyp:ft2',
					'deps/ffmpeg/ffmpeg.gyp:ffmpeg_compile',
					'deps/libgif/libgif.gyp:libgif', 
					'deps/libjpeg/libjpeg.gyp:libjpeg', 
					'deps/libpng/libpng.gyp:libpng',
					'deps/libwebp/libwebp.gyp:libwebp',
					'deps/tinyxml2/tinyxml2.gyp:tinyxml2',
					'deps/v8-link/v8-link.gyp:v8-link',
					'deps/v8-link/v8-link.gyp:v8_libplatform-link',
					'deps/node/deps/uv/uv.gyp:libuv',
					'deps/node/deps/openssl/openssl.gyp:openssl',
					'deps/node/deps/http_parser/http_parser.gyp:http_parser',
					'deps/node/node.gyp:node',
					'deps/bplus/bplus.gyp:bplus',
				],
				'sources': [ '../tools/useless.c' ],
				'link_settings': { 
					'libraries': [ '-lz' ],
				},
				'ldflags': [
					'-s',
					'-Wl,--whole-archive',
					'<(output)/obj.target/ffmpeg/libffmpeg.a',
					'-Wl,--no-whole-archive',
				],
			},
			{
				'target_name': 'flare-depes-copy',
				'type': 'none',
				'dependencies': [ 'flare-depes-test' ],
				'copies': [{
					'destination': '<(DEPTH)/out/jniLibs/<(android_abi)',
					'files': [
						'<(output)/lib.target/libflare-depes-test.so',
					],
				}],
			}],
		}],
		['os in "ios osx"', {
			'targets': [
			{
				'target_name': 'FlareTest',
				'type': 'shared_library',
				'mac_bundle': 1,
				'include_dirs': [ '.' ],
				'direct_dependent_settings': {
					'include_dirs': [ '.' ],
				},
				'sources': [
					'framework/framework.h',
					'framework/Thing.h',
					'framework/Thing.m',
					'framework/Info-<(os).plist',
				],
				'link_settings': {
					'libraries': [
						'$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
					],
				},
				'mac_framework_headers': [
					'framework/framework.h',
					'framework/Thing.h',
				],
				'xcode_settings': {
					'INFOPLIST_FILE': '<(DEPTH)/test/framework/Info-<(os).plist',
					#'SKIP_INSTALL': 'NO',
					'LD_RUNPATH_SEARCH_PATHS': [
						'$(inherited)',
						'@executable_path/Frameworks',
						'@loader_path/Frameworks',
					],
					'DYLIB_INSTALL_NAME_BASE': '@rpath',
				},
			}],
		}]
	],
}
