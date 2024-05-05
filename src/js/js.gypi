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
			'../../deps/libuv/include',
		],
		'dependencies': [
			'quark',
			'build_libs_quark_',
		],
		'sources': [
			'js_.h',
			'js.h',
			'js.cc',
			'types.h',
			'types.cc',
			'wrap.cc',
			'api/app.cc', # api
			'api/action.cc',
			'api/app.cc',
			'api/box.cc',
			'api/buffer.cc',
			'api/cb.cc',
			'api/cb.h',
			'api/console.cc',
			'api/css.cc',
			'api/event.cc',
			'api/fs_path.cc',
			'api/fs_reader.cc',
			'api/fs.cc',
			'api/http.cc',
			'api/json.cc',
			'api/os.cc',
			'api/screen.cc',
			'api/scroll.cc',
			'api/storage.cc',
			'api/text.cc',
			'api/timer.cc',
			'api/transform.cc',
			'api/util.cc',
			'api/view.cc',
			'api/view.h',
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
				'sources': [ 'link_v8.cc' ],
			}],
			['v8_enable_inspector==1', { 'defines': [ 'HAVE_INSPECTOR=1' ] }],
		],
		# actions
		'actions': [
			{
				'action_name': 'gen_inl_js_natives',
				'inputs': [
					'../../tools/gen-js-natives.js',
					'../../out/_pkg.js',
					'../../out/_pkgutil.js',
					'../../out/_event.js',
					'../../out/_types.js',
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
					'',
					'<@(_outputs)',
				],
				'process_outputs_as_sources': 1,
			},
		],
	},
	{
		'target_name': 'build_libs_quark_',
		'type': 'none',
		'actions': [{
			'action_name': 'build',
			'inputs': [
				'<@(libs_quark_ts_in)',
				# '<!(node -e \'fs.readdirSync("libs/quark").filter(e=>require("path")'
				# '.extname(e).match(/\.(ts|js|tsx|json)$/)).map(e=>`libs/quark/${e}`)\')',
			],
			'outputs': [
				'../../libs/quark/out/files.gypi',
			],
			'action': [ 'sh', '-c', 'cd libs/quark; npm run build' ],
		}],
	}
	],

	'conditions': [
		['os!="ios"', {
			'targets+': [
			{
				'target_name': 'quark-exec',
				'product_name': 'quark',
				'type': 'executable',
				'dependencies': [
					'quark',
					'quark-js',
					# 'quark-media',
				],
				'sources': [
					'main.cc',
				],
				'ldflags': [ '<@(other_ldflags)' ],
			}],
		}]
	],
}