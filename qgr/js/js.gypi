{
	'variables': {
		'use_v8%': 0,
	},
	'targets': [{
		'target_name': 'qgr-js',
		'type': '<(library)',
		'include_dirs': [
			'../../out', 
			'../..',
			#'../../node/deps/cares/include',
			#'../../node/deps/uv/include',
		],
		'dependencies': [
			'qgr-utils', 
			'qgr-gui', 
			#'node/deps/openssl/openssl.gyp:openssl',
			'v8-link/v8-link.gyp:v8-link',
			'v8-link/v8-link.gyp:v8_libplatform-link',
		],
		'direct_dependent_settings': {
			'include_dirs': [ '../..' ],
			'mac_bundle_resources': [
				#'../../node_modules/qgr',
			],
		},
		'defines': [ 'NODE_WANT_INTERNALS=1' ],
		'sources': [
			'../../out/native-core-js.cc',
			'js-1.h',
			'js-cls.cc',
			'js.cc',
			'js.h',
			'qgr.h',
			'qgr.cc',
			'rlog.cc',
			'str.h',
			'str.cc',
			'v8.cc',
			'value.h',
			'wrap.h',
			'wrap.cc',
			# binding qgr-utils
			'binding/cb-1.h',
			'binding/cb.cc',
			'binding/fs-1.h',
			'binding/fs.cc',
			'binding/fs-reader.cc',
			'binding/fs-path.cc',
			'binding/http.cc',
			'binding/util.cc',
			'binding/storage.cc',
			'binding/json-1.h',
			'binding/json.cc',
			'binding/event-1.h',
			'binding/event.cc',
			'binding/sys.cc',
			'binding/event.cc',
			'binding/timer.cc',
			'binding/console.cc',
			# binding qgr-gui
			'binding/value.cc',
			'binding/qgr-binding.cc',
			'binding/action.cc',
			'binding/action-frame.cc',
			'binding/app.cc',
			'binding/audio-player.cc',
			'binding/video.cc',
			'binding/media.cc',
			'binding/div.cc',
			'binding/display-port.cc',
			'binding/indep.cc',
			'binding/image.cc',
			'binding/layout.cc',
			'binding/box.cc',
			'binding/view.cc',
			'binding/root.cc',
			'binding/span.cc',
			'binding/sprite.cc',
			'binding/hybrid.cc',
			'binding/text-font.cc',
			'binding/text-node.cc',
			'binding/label.cc',
			'binding/limit.cc',
			'binding/panel.cc',
			'binding/button.cc',
			'binding/scroll.cc',
			'binding/css.cc',
			'binding/font.cc',
			'binding/text.cc',
			'binding/input.cc',
			'binding/background.cc',
		],
		'conditions': [
			['v8_enable_inspector==1', { 'defines': [ 'HAVE_INSPECTOR=1' ] }],
			['node_use_openssl=="true"', { 'defines': [ 'HAVE_OPENSSL=1' ] }],
			['node_use_dtrace=="true"', { 'defines': [ 'HAVE_DTRACE=1' ] }],
		],
		'actions': [
			{
				'variables': {
					'native_core_js_files': [
					'binding/ext.js',
					'binding/event.js',
					'binding/value.js',
					],
				},
				'action_name': 'gen_core_js_natives',
				'inputs': [
					'../../tools/gen-js-natives.js',
					'<@(native_core_js_files)',
				],
				'outputs': [
					'../../out/native-core-js.h',
					'../../out/native-core-js.cc',
				],
				'action': [
					'<(node)',
					'<@(_inputs)',
					'CORE',
					'wrap',
					'<@(_outputs)',
				],
			},
		],
	}]
}