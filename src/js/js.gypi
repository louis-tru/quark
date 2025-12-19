{
	'includes': [
		'../../libs/quark/out/files.gypi',
	],
	'targets': [
	{
		'target_name': 'quark-js',
		'type': 'static_library',
		'include_dirs': [
			'../../out',
			'../../deps/http_parser',
			'../../deps/libuv/include',
			'../../deps/openssl/openssl/include',
		],
		'dependencies': [
			'quark',
			'quark-media',
			'build_libs_quark',
			'deps/spine/spine.gyp:spine',
		],
		'direct_dependent_settings': {
			'defines': [ 'USE_JS=1' ],
		},
		'sources': [
			'cb.cc',
			'init.cc',
			'js_.h',
			'js.h',
			'js.cc',
			'cls.cc',
			'mix.cc',
			'api/action.cc', # api
			'api/box.cc',
			'api/buffer.cc',
			'api/css.cc',
			'api/event.cc',
			'api/font.cc',
			'api/filter.cc',
			'api/fs.cc',
			'api/http.cc',
			'api/net.cc',
			'api/os.cc',
			'api/player.cc',
			'api/screen.cc',
			'api/scroll.cc',
			'api/storage.cc',
			'api/sprite.cc',
			'api/spine.cc',
			'api/text.cc',
			'api/morph.cc',
			'api/types.h',
			'api/types.cc',
			'api/ui.cc',
			'api/ui.h',
			'api/view.cc',
			'api/window.cc',
			'api/path.cc',
			'api/lmdb.cc',
		],
		'conditions': [
			['use_v8==0 and os in "mac ios"', { # use javascriptcore
				'link_settings': {
					'libraries': [
						'$(SDKROOT)/System/Library/Frameworks/JavaScriptCore.framework',
					]
				},
				'direct_dependent_settings': {
					'defines': [ 'USE_JSC=1' ],
				},
				'defines': [ 'USE_JSC=1' ],
				'sources': [
					'jsc/exceptions.cc',
					'jsc/handle_scope.cc',
					'jsc/jsc.cc',
					'jsc/jsc.h',
					'jsc/jsccls.cc',
					'jsc/macros.h',
				],
			}, { # use v8
				'dependencies': [
					# 'tools/v8_gypfiles/d8.gyp:d8'
					'tools/v8_gypfiles/v8.gyp:v8_maybe_snapshot',
					'tools/v8_gypfiles/v8.gyp:v8_libplatform',
				],
				'sources': [
					'v8/inspector_agent.cc',
					'v8/inspector_agent.h',
					'v8/inspector_io.cc',
					'v8/inspector_io.h',
					'v8/inspector_socket_server.cc',
					'v8/inspector_socket_server.h',
					'v8/inspector_socket.cc',
					'v8/inspector_socket.h',
					'v8/v8js.h',
					'v8/v8js.cc',
					'v8/v8cls.cc',
				],
				'actions': [{
					'action_name': 'v8_inspector_compress_protocol_json',
					'process_outputs_as_sources': 1,
					'inputs': [
						'../../deps/v8/include/js_protocol-1.3.json',
					],
					'outputs': [
						'../../out/v8_inspector_protocol_json.h',
					],
					'action': [
						'<(python)',
						'tools/compress_json.py',
						'<@(_inputs)',
						'<@(_outputs)',
					],
				}],
			}],
		],
		'actions': [
			{
				'action_name': 'gen_inl_js_natives',
				'inputs': [
					'../../tools/gen_js_natives.js',
					'../../libs/quark/out/_pkg.js',
					'../../libs/quark/out/_util.js',
					'../../libs/quark/out/_event.js',
					'../../libs/quark/out/_types.js',
					'../../libs/quark/out/_ext.js',
					'../../libs/quark/out/_uri.js',
				],
				'outputs': [
					'../../out/native-inl-js.h',
					'../../out/native-inl-js.cc',
				],
				'action': [
					'<(node)',
					'<@(_inputs)',
					'',
					'INL',
					'wrap', # wrap code
					'<@(_outputs)',
				],
				'process_outputs_as_sources': 1,
			},
			{
				'action_name': 'gen_lib_js_natives',
				'inputs': [
					'../../tools/gen_js_natives.js',
					'<@(libs_quark_js_out)',
				],
				'outputs': [
					'../../out/native-lib-js.h',
					'../../out/native-lib-js.cc',
				],
				'action': [
					'<(node)',
					'<@(_inputs)',
					'quark', # pkgname prefix
					'LIB', # namespace prefix
					'wrap',
					'<@(_outputs)',
				],
				'process_outputs_as_sources': 1,
			},
		],
	},
	{ # build quark ts
		'target_name': 'build_libs_quark',
		'type': 'none',
		'actions': [{
			'action_name': 'build',
			'inputs': [ '<@(libs_quark_ts_in)' ],
			'outputs': [ '../../libs/quark/out/files.gypi' ],
			'action': [ 'sh', '-c', 'cd libs/quark; npm run build' ],
		}],
	}],
}