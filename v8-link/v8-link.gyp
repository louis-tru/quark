{
	'variables': {
		'library%': 'static_library',
		'OS%': 'mac',
		'use_v8%': 1,
		'v8_source%': '../node/deps/v8',
		'v8_includes': [
			'include/v8.h',
			'include/v8.h.inl',
			'include/v8config.h',
			'include/v8-version.h',
			'include/v8-version-string.h',
			'include/v8-value-serializer-version.h',
			'include/v8-util.h',
			'include/v8-testing.h',
			'include/v8-platform.h',
			'include/v8-profiler.h',
			'include/v8-inspector.h',
			'include/v8-inspector-protocol.h',
			'include/v8-debug.h',
			'include/libplatform/libplatform-export.h',
			'include/libplatform/libplatform.h',
			'include/libplatform/v8-tracing.h',
		],
	},
	'target_defaults': {
		'direct_dependent_settings': {
	    'include_dirs': [ 'include', ],
	  },
	},
	'conditions': [
		['use_v8==0 and os=="ios"', {
			'target_defaults': {
        'defines': [ 'USE_JSC=1' ],
        'direct_dependent_settings': {
          'defines': [ 'USE_JSC=1' ],
        },
			},
			'targets': [
			{
				'target_name': 'v8-link',
				'type': '<(library)',
				'include_dirs': [ 'include', ],
        'link_settings': {
          'libraries': [
            '$(SDKROOT)/System/Library/Frameworks/JavaScriptCore.framework',
          ]
        },
				'sources': [
					'<@(v8_includes)',
					'util.h',
					'util.cc',
					'native-js.cc',
					'jsc-v8.cc',
          'jsc-v8-serializer.cc.inl',
          'jsc-v8-script.cc.inl',
          'jsc-v8-exceptions.cc.inl',
          'jsc-v8-property-descriptor.cc.inl',
          'jsc-v8-check.cc.inl',
          'jsc-v8-template.cc.inl',
          'jsc-v8-value.cc.inl',
          'jsc-v8-object.cc.inl',
          'jsc-v8-function.cc.inl',
          'jsc-v8-string.cc.inl',
          'jsc-v8-array-buffer.cc.inl',
          'jsc-v8-promise.cc.inl',
          'jsc-v8-unimplemented.cc.inl',
          'jsc-v8-wrap.cc.inl',
          'jsc-v8-isolate.cc.inl',
          'jsc-v8-context.cc.inl',
					'debug.cc',
					'inspector.cc',
					'libplatform.cc',
					'testing.cc',
					'tracing.cc',
					'profiler.cc',
				],
        'actions': [
          {
            'variables': {
              'native_js_files': [
                'jsc-v8-isolate.js',
                'jsc-v8-context.js',
              ],
            },
            'action_name': 'gen_js_natives',
            'inputs': [
              'gen-js-natives.js',
              '<@(native_js_files)',
            ],
            'outputs': [
              'native-js.h',
              'native-js.cc',
            ],
            'action': [
              'node',
              '<@(_inputs)',
              'JSC',
              'wrap',
              '<@(_outputs)',
            ],
          },
        ],
			},
			{
				'target_name': 'v8_libplatform-link',
				'type': 'none', 
			},
			{
				'target_name': 'postmortem-metadata-link',
				'type': 'none',
			},],
		}, 
		{ # use v8
			'target_defaults': {
        'defines': [ 'USE_JSC=0' ],
        'direct_dependent_settings': {
          'defines': [ 'USE_JSC=0' ],
        },
			},
			'targets': [
			{
				'target_name': 'v8-link',
				'type': 'none',
				'sources': [
					'<@(v8_includes)',
				],
				'dependencies': ['<(v8_source)/src/v8.gyp:v8'],
			},
			{
				'target_name': 'v8_libplatform-link',
				'type': 'none',
				'dependencies': ['<(v8_source)/src/v8.gyp:v8_libplatform'],
			},
			{
				'target_name': 'postmortem-metadata-link',
				'type': 'none',
				'dependencies': ['<(v8_source)/src/v8.gyp:postmortem-metadata'],
			},]
		}],
	],
}
