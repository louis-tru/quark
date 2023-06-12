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
				'../deps/openssl/openssl/include',
			],
			'dependencies': [
				'quark-util',
				'quark',
				# 'quark-media',
				# 'quark-js',
				'trial',
				'deps/ffmpeg/ffmpeg.gyp:ffmpeg',
				'deps/freetype/freetype.gyp:freetype',
			],
			'mac_bundle': 1,
			'mac_bundle_resources': [
				'res',
				'test-quark',
				'../examples',
				'../bench',
			],
			'xcode_settings': {
				'OTHER_LDFLAGS': ['-all_load'],
			},
			'sources': [
				'../examples',
				'../libs/qkmake',
				'../libs/somes',
				'test-alsa-ff.cc',
				# 'test-benchmark.cc',
				'test-buffer.cc',
				# 'test-demo.cc',
				'test-event.cc',
				'test-ffmpeg.cc',
				# 'test-font.cc',
				# 'test-freetype.cc',
				'test-fs-async.cc',
				'test-fs.cc',
				'test-fs2.cc',
				# 'test-gui.cc',
				'test-http-cookie.cc',
				'test-http.cc',
				'test-http2.cc',
				'test-http3.cc',
				'test-https.cc',
				'test-ios-run-loop.cc',
				'test-jsc.cc',
				'test-json.cc',
				'test-jsx.cc',
				'test-layout.cc',
				'test-canvas.cc',
				'test-canvas1.cc',
				'test-linux-input-2.cc',
				'test-linux-input.cc',
				'test-list.cc',
				'test-localstorage.cc',
				'test-loop.cc',
				'test-map.cc',
				'test-mutex.cc',
				'test-net-ssl.cc',
				'test-net.cc',
				'test-number.cc',
				'test-device.cc',
				'test-sizeof.cc',
				'test-ssl.cc',
				'test-string.cc',
				'test-thread.cc',
				'test-util.cc',
				'test-uv.cc',
				# 'test-v8.cc',
				'test-zlib.cc',
				'test.cc',
			],
			'conditions': [
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
		# gen android test depes `libquark-depes-test.so`
		['os=="android" and (debug==1 or without_visibility_hidden==1)', {
			'targets': [
			{
				'target_name': 'quark-depes-test',
				'type': 'shared_library',
				'dependencies': [
					'deps/zlib/minizip.gyp:minizip',
					'deps/libtess2/libtess2.gyp:libtess2',
					'deps/freetype2/freetype2.gyp:ft2',
					'deps/ffmpeg/ffmpeg.gyp:ffmpeg_compile',
					'deps/libgif/libgif.gyp:libgif', 
					'deps/libjpeg/libjpeg.gyp:libjpeg', 
					'deps/libpng/libpng.gyp:libpng',
					'deps/libwebp/libwebp.gyp:libwebp',
					'deps/tinyxml2/tinyxml2.gyp:tinyxml2',
					'deps/libuv/libuv.gyp:libuv',
					'deps/openssl/openssl.gyp:openssl',
					'deps/http_parser/http_parser.gyp:http_parser',
					'deps/libbptree/libbptree.gyp:libbptree',
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
				'target_name': 'quark-depes-copy',
				'type': 'none',
				'dependencies': [ 'quark-depes-test' ],
				'copies': [{
					'destination': '<(DEPTH)/out/jniLibs/<(android_abi)',
					'files': [
						'<(output)/lib.target/libquark-depes-test.so',
					],
				}],
			}],
		}],
		['os in "ios osx"', {
			'targets': [
			{
				'target_name': 'QuarkTest',
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
