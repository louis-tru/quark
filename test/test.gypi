{ 
	'variables': {
		'without_visibility_hidden%': 0,
		'other_ldflags': [
			'-Wl,--whole-archive',
			'<(output)/obj.target/libngui-utils.a',
			'<(output)/obj.target/libngui-gui.a',
			'<(output)/obj.target/libngui-js.a',
			'<(output)/obj.target/node/libnode.a',
			# '<(output)/obj.target/node/deps/v8/src/libv8_base.a',
			# '<(output)/obj.target/node/deps/openssl/libopenssl.a',
			'-Wl,--no-whole-archive',
		],
	},
	'targets': [
		{
			'target_name': 'test',
			'type': 'executable',
			'include_dirs': [ 
				'../out',
				'../node/src',
			],
			'dependencies': [
				'ngui-lib',
				'ngui-utils',
				'ngui-gui',
				'ngui-js',
				'depe/tess2/tess2.gyp:tess2',
				'depe/freetype2/freetype2.gyp:ft2',
				'depe/curl/curl.gyp:curl',
				'depe/ffmpeg/ffmpeg.gyp:ffmpeg',
				'node/deps/openssl/openssl.gyp:openssl',
				'v8-link/v8-link.gyp:v8-link',
				'v8-link/v8-link.gyp:v8_libplatform-link',
				'node/deps/uv/uv.gyp:libuv',
				'node/deps/http_parser/http_parser.gyp:http_parser',
				'node/node.gyp:node', 
			],
			'mac_bundle': 1,
			'mac_bundle_resources': [
				'res',
				'test-ngui',
				'../node_modules/ngui',
				'../demo/examples',
				'../benchmark',
			],
			'xcode_settings': {
				'OTHER_LDFLAGS': '-all_load',
			},
			'sources': [
				'../demo/examples',
				'../demo/README.md',
				'../node_modules/ngui',
				'../node_modules/ngui-tools',
				'../node_modules/ngui-utils',
				'test.cc',
				'test-ngui.cc',
				'test-fs.cc',
				'test-fs2.cc',
				'test-gui.cc',
				'test-freetype.cc',
				'test-json.cc',
				'test-string.cc',
				'test-list.cc',
				'test-map.cc',
				'test-event.cc',
				'test-zlib.cc',
				'test-curl.cc',
				'test-curl2.cc',
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
				'test-demo.cc',
				'test-jsc.cc',
				'test-v8.cc',
				'test-loop.cc',
				'test-sys.cc',
				'test-mutex.cc',
				'test-ios-run-loop.cc', 
				'test-benchmark.cc',
				'test-sizeof.cc',
				'test-util.cc',
				'test-alsa-ff.cc',
			],
			'conditions': [
				['os in "ios osx"', {
					'sources': [ 
						'test-<(os).plist',
						'Storyboard-<(os).storyboard',
					],
					'xcode_settings': {
						'INFOPLIST_FILE': '$(SRCROOT)/test/test-<(os).plist',
					},
				}],
				['os in "linux android"', {
					'ldflags': [ '<@(other_ldflags)' ],
				}],
			],
		},
		{
			'target_name': 'test2',
			'type': 'executable',
			'sources': [
				'test2.cc',
				'test2-thread.cc',
				'test2-x11.cc',
				'test2-xim.cc',
				'test2-alsa.cc',
				'test2-alsa2.cc',
				'test2-xopen2.cc',
			],
			'mac_bundle': 1,
			'xcode_settings': {
				'OTHER_LDFLAGS': '-all_load',
			},
			'conditions': [
				['os in "ios osx"', {
					'sources': [
						'test-<(os).plist',
						'Storyboard-<(os).storyboard',
					],
					'xcode_settings': {
						'INFOPLIST_FILE': '$(SRCROOT)/test/test-<(os).plist',
					},
				}],
				['os in "linux"', {
					'link_settings': { 
						'libraries': [ 
							'-lGLESv2', '-lEGL', '-lX11', '-lasound',
						],
					},
				}],
			],
		},
		{
			'target_name': 'ngui-demo',
			'type': 'executable',
			'dependencies': [ 
				'ngui-lib',
			],
			'mac_bundle': 1,
			'mac_bundle_resources': [
				'../demo/examples',
				'../node_modules/ngui',
			],
			'xcode_settings': {
				'OTHER_LDFLAGS': '-all_load',
			},
			'sources': [
				'demo.cc',
			],
			'conditions': [
				['os in "ios osx"', {
					'sources': [
						'test-<(os).plist',
						'Storyboard-<(os).storyboard',
					],
					'xcode_settings': {
						'INFOPLIST_FILE': '$(SRCROOT)/test/test-<(os).plist',
					},
				}],
				['os in "linux android"', {
					'ldflags': [ '<@(other_ldflags)' ],
				}],
			],
		},
	],

	'conditions': [
		['os=="android" and (debug==1 or without_visibility_hidden==1)', {
			'targets': [
			{
				'target_name': 'ngui_depes',
				'type': 'shared_library',
				'dependencies': [
					'depe/curl/curl.gyp:curl',
					'ngui/utils/minizip.gyp:minizip',
					'depe/sqlite-amalgamation/sqlite3.gyp:sqlite3',
					'depe/tess2/tess2.gyp:tess2', 
					'depe/freetype2/freetype2.gyp:ft2',
					'depe/ffmpeg/ffmpeg.gyp:ffmpeg_compile',
					'depe/libgif/libgif.gyp:libgif', 
					'depe/libjpeg-turbo/libjpeg.gyp:libjpeg', 
					'depe/libpng/libpng.gyp:libpng',
					'depe/libwebp/libwebp.gyp:libwebp',
					'depe/tinyxml2/tinyxml2.gyp:tinyxml2',
					'node/node.gyp:node',
				],
				'link_settings': { 'libraries': [ '-lz' ] },
				'ldflags': [ 
					'-s',
					'-Wl,--whole-archive',
					'<(output)/obj.target/ffmpeg/libffmpeg.a',
					'-Wl,--no-whole-archive',
				],
				'direct_dependent_settings': {
					'include_dirs': [ 
						'../node/deps/v8/include', 
						'../node/deps/http_parser', 
						'../node/deps/uv/include', 
						'../node/deps/openssl/openssl/include', 
						'../depe/ffmpeg', 
						'../depe/tinyxml2', 
						'../depe/tess2/include', 
						'../depe/sqlite-amalgamation', 
						'../depe/freetype2/include', 
						'../depe/libgif/lib', 
						'../depe/libjpeg-turbo', 
						'../depe/libpng', 
						'../depe/libwebp', 
						'../depe/rapidjson/include',
					],
					'defines': [ 
						'__STDC_CONSTANT_MACROS', 
						'CHROME_PNG_WRITE_SUPPORT',
						'PNG_USER_CONFIG',
					],
				},
			}, {
				'target_name': 'ngui_depes_copy',
				'type': 'none',
				'dependencies': [ 'ngui_depes' ],
				'copies': [{
					'destination': '<(DEPTH)/out/jniLibs/<(android_abi)',
					'files': [
						'<(output)/lib.target/libngui_depes.so',
					],
				}],
			}],
		}],
		['os in "ios osx"', {
			'targets': [
			{
				'target_name': 'NguiTest',
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
