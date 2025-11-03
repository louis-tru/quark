{
	'variables': {
		'without_visibility_hidden%': 0,
	},
	'targets': [{
		'target_name': 'test1',
		'type': 'executable',
		'include_dirs': [
			'../out',
			'../deps/openssl/openssl/include',
			'../deps/v8/include',
		],
		'dependencies': [
			'quark-util',
			'quark',
			'quark-media',
			'quark-js',
			'trial',
			'deps/ffmpeg/ffmpeg.gyp:ffmpeg',
			'deps/freetype/freetype.gyp:freetype',
		],
		'sources': [
			'../libs/qkmake',
			'../libs/qktool',
			'util/test-event.cc',
			'util/test-fs-async.cc',
			'util/test-fs.cc',
			'util/test-fs2.cc',
			'util/test-buffer.cc',
			'util/test-http-cookie.cc',
			'util/test-http.cc',
			'util/test-http2.cc',
			'util/test-http3.cc',
			'util/test-https.cc',
			'util/test-ios-run-loop.cc',
			'util/test-json.cc',
			'util/test-list.cc',
			'util/test-localstorage.cc',
			'util/test-loop.cc',
			'util/test-map.cc',
			'util/test-mutex.cc',
			'util/test-net-ssl.cc',
			'util/test-net.cc',
			'util/test-number.cc',
			'util/test-os-info.cc',
			'util/test-sizeof.cc',
			'util/test-ssl.cc',
			'util/test-string.cc',
			'util/test-thread.cc',
			'util/test-util.cc',
			'util/test-uv.cc',
			'util/test-zlib.cc',
			'util/test-atomic.cc',
			'test-freetype.cc',
			'test-gui.cc',
			'test-alsa-ff.cc',
			'test-ffmpeg.cc',
			'test-media.cc',
			'test-layout.cc',
			'test-canvas.cc',
			'test-rrect.cc',
			'test-draw-efficiency.cc',
			'test-blur.cc',
			'test-subcanvas.cc',
			'test-outimg.cc',
			'test-linux-input.cc',
			'test-css.cc',
			'test-action.cc',
			'test-openurl.cc',
			'test-input.cc',
			'test-jsx.cc',
			'test-v8.cc',
			'test-jsc.cc',
			'test-spine.cc',
			'test-little_border.cc',
			'test-mtv.cc',
			'test.cc',
			'test.h',
		],
		'conditions': [
			['os in "mac ios"', {
				'sources': [
					'test-<(os).plist',
				],
				'xcode_settings': {
					# 'OTHER_LDFLAGS': '-all_load',
					# 'OTHER_LDFLAGS': '-static-libstdc++ -static-libgcc',
					'INFOPLIST_FILE': '<(output)/../../test/test-<(os).plist',
					'SKIP_INSTALL': 'NO',
					'ASSETCATALOG_COMPILER_APPICON_NAME': 'AppIcon',
					# 'ASSETCATALOG_COMPILER_LAUNCHIMAGE_NAME': 'LaunchImage',
					'PRODUCT_BUNDLE_IDENTIFIER': 'org.quark.test1',
					'IPHONEOS_DEPLOYMENT_TARGET': '<(version_min)',
				},
				'link_settings': {
					'libraries': [
						'$(SDKROOT)/System/Library/Frameworks/JavaScriptCore.framework',
					],
				},
				'mac_bundle': 1,
				'mac_bundle_resources': [
					'./Storyboard-<(os).storyboard',
					'./Images.xcassets',
					'./launch/launch.png',
				],
			}],
			['use_js==1', { # use jsapi
				'conditions': [
					['os in "mac ios"', { # mac ios
						'mac_bundle_resources': [
							'./jsapi/out/jsapi',
							'../examples/out/examples',
						],
					}, { # android linux
						'copies': [{
							'destination': '<(output)',
							'files': [
								'./jsapi/out/jsapi',
								'../examples/out/examples',
							],
						}], # copy
					}],
				],
				'actions': [{
					'action_name': 'build_jsapi',
					'inputs': [ '<@(jsapi_in)', '<@(examples_in)' ],
					'outputs': [
						'test-jsapi.cc',
						'./jsapi/out/files.gypi',
						'../examples/out/files.gypi',
					],
					'action': [
						'sh', '-c',
						'cd test/jsapi && '
						'npm run build && '
						'cd ../../examples && '
						'tsc && node _'
					],
					'process_outputs_as_sources': 1,
				}],
			}, { # no jsapi
				'actions': [],
				'sources+': ['test-jsapi.cc'],
			}], # ['use_js==1'
		],
	}],

	# tests
	'conditions': [
		['use_js==1', {
			'includes': [
				'./jsapi/out/files.gypi',
				'../examples/out/files.gypi',
			],
		}],
		# gen android test deps `libquark-deps-test.so`
		['os=="android" and (debug==1 or without_visibility_hidden==1)', {
			'targets+': [
			{
				'target_name': 'quark_deps_test',
				'type': 'shared_library',
				'dependencies': [
					'deps/bptree/bptree.gyp:bptree',
					'deps/ffmpeg/ffmpeg.gyp:ffmpeg_compile',
					'deps/freetype/freetype.gyp:freetype',
					'deps/http_parser/http_parser.gyp:http_parser',
					'deps/libexpat/libexpat.gyp:libexpat',
					'deps/libgif/libgif.gyp:libgif',
					'deps/libjpeg/libjpeg.gyp:libjpeg',
					'deps/libpng/libpng.gyp:libpng',
					'deps/libtess2/libtess2.gyp:libtess2',
					'deps/libuv/libuv.gyp:libuv',
					'deps/libwebp/libwebp.gyp:libwebp',
					'deps/openssl/openssl.gyp:openssl',
					'deps/tinyxml2/tinyxml2.gyp:tinyxml2',
					'deps/zlib/minizip.gyp:minizip',
				],
				'sources': [
					'../tools/useless.c',
				],
				'link_settings': {
					'libraries': [ '-lz' ],
				},
				'ldflags': [
					'-s',
					'-Wl,--version-script,<(source)/tools/v_all.ver',
					'-Wl,--whole-archive',
					'<(output)/obj.target/ffmpeg/libffmpeg.a',
					'-Wl,--no-whole-archive',
				],
				'conditions': [['use_js==1', {
					'dependencies': [
						'tools/v8_gypfiles/v8.gyp:v8_maybe_snapshot',
						'tools/v8_gypfiles/v8.gyp:v8_libplatform',
					],
				}]],
			},
			{
				'target_name': 'quark_deps_copy',
				'type': 'none',
				'dependencies': [ 'quark_deps_test' ],
				'copies': [{
					'destination': '<(DEPTH)/out/jniLibs/<(android_abi)',
					'files': [ '<(output)/obj.target/libquark_deps_test.so' ],
				}],
			}],
		}],
		['os in "mac ios"', {
			'targets+': [ # test macos framework
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
