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
			'_js.h',
			'js.cc',
			'js.h',
			'start.cc',
			'types.h',
			'types.cc',
			'wrap.cc',
			# api 
			'api/_cb.h',
			'api/_fs.h',
			'api/_json.h',
			'api/action-frame.cc',
			'api/action.cc',
			'api/app.cc',
			'api/audio-player.cc',
			'api/background.cc',
			'api/binding.cc',
			'api/box.cc',
			'api/buffer.cc',
			'api/cb.cc',
			'api/console.cc',
			'api/css.cc',
			'api/display-port.cc',
			'api/div.cc',
			'api/event.cc',
			'api/font.cc',
			'api/fs-path.cc',
			'api/fs-reader.cc',
			'api/fs.cc',
			'api/http.cc',
			'api/hybrid.cc',
			'api/image.cc',
			'api/indep.cc',
			'api/input.cc',
			'api/json.cc',
			'api/label.cc',
			'api/layout.cc',
			'api/limit.cc',
			'api/media.cc',
			'api/os.cc',
			'api/root.cc',
			'api/scroll.cc',
			'api/span.cc',
			'api/sprite.cc',
			'api/storage.cc',
			'api/text-font.cc',
			'api/text-node.cc',
			'api/text.cc',
			'api/timer.cc',
			'api/util.cc',
			'api/video.cc',
			'api/view.h',
			'api/view.cc',
		],
		'conditions': [
			['use_v8==0 and OS==mac', { # use javascriptcore
				'link_settings': {
					'libraries': [
						'$(SDKROOT)/System/Library/Frameworks/JavaScriptCore.framework',
					]
				},
				'defines': [ 'USE_JSC=1' ],
				'sources': [ 'link_jsc.cc' ],
			}, { # use v8
				'dependencies': [
					'deps/v8/src/v8.gyp:v8',
					'deps/v8/src/v8.gyp:v8_libplatform',
					# 'deps/v8/src/v8.gyp:postmortem-metadata',
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
			],
			'outputs': [
				'../out/_pkg.js',
				'../out/_pkgutil.js',
				'../out/_event.js',
				'../out/_value.js',
				# '<@(libs_quark_js_out)',
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
					'quark-media',
				],
				'sources': [
					'main.cc',
				],
				'ldflags': [ '<@(other_ldflags)' ],
			}],
		}]
	],
}