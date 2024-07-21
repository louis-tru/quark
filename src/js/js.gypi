{
	'includes': [
		'../../libs/quark/out/files.gypi',
	],
	'targets': [
	{
		'target_name': 'quark-js',
		'type': 'static_library', #<(output_type)
		'include_dirs': [
			'../../out',
			'../../deps/http_parser',
			'../../deps/libuv/include',
			'../../deps/openssl/openssl/include',
		],
		'dependencies': [
			'quark',
			'build_libs_quark_',
		],
		'sources': [
			'cb.cc',
			'init.cc',
			'js_.h',
			'js.h',
			'js.cc',
			'wrap.cc',
			'api/action.cc', # api
			'api/box.cc',
			'api/buffer.cc',
			'api/css.cc',
			'api/event.cc',
			'api/font.cc',
			'api/filter.cc',
			'api/fs.cc',
			'api/http.cc',
			'api/os.cc',
			'api/screen.cc',
			'api/scroll.cc',
			'api/storage.cc',
			'api/text.cc',
			'api/matrix.cc',
			'api/types.h',
			'api/types.cc',
			'api/ui.cc',
			'api/ui.h',
			'api/view.cc',
			'api/window.cc',
		],
		'conditions': [
			['use_v8==0 and OS=="mac"', { # use javascriptcore
				'link_settings': {
					'libraries': [
						'$(SDKROOT)/System/Library/Frameworks/JavaScriptCore.framework',
					]
				},
				'defines': [ 'USE_JSC=1' ],
				'sources': [ 'link_jsc.cc' ],
			}, { # use v8
				'dependencies': [
					'tools/v8_gypfiles/v8.gyp:v8_maybe_snapshot',
					'tools/v8_gypfiles/v8.gyp:v8_libplatform',
					# 'tools/v8_gypfiles/d8.gyp:d8'
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
					'v8/v8.cc',
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
						'python',
						'tools/compress_json.py',
						'<@(_inputs)',
						'<@(_outputs)',
					],
				}],
			}],
			['v8_enable_inspector==1', { 'defines': [ 'HAVE_INSPECTOR=1' ] }],
		],
		# actions
		'actions': [
			{
				'action_name': 'gen_inl_js_natives',
				'inputs': [
					'../../tools/gen-js-natives.js',
					'../../libs/quark/out/_pkg.js',
					'../../libs/quark/out/_util.js',
					'../../libs/quark/out/_event.js',
					'../../libs/quark/out/_types.js',
					'../../libs/quark/out/_ext.js',
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
				'action_name': 'gen_lib_js_natives_',
				'inputs': [
					'../../tools/gen-js-natives.js',
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
		'target_name': 'build_libs_quark_',
		'type': 'none',
		'actions': [{
			'action_name': 'build',
			'inputs': [ '<@(libs_quark_ts_in)' ],
			'outputs': [ '../../libs/quark/out/files.gypi' ],
			'action': [ 'sh', '-c', 'cd libs/quark; npm run build' ],
		}],
	}],
}