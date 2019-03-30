{
	'includes': [
		'qgr/utils/utils.gypi',
		'qgr/qgr.gypi',
		'qgr/js/js.gypi',
		'tools/default_target.gypi',
	],

	'variables': {
		'more_log%': 0,
		'qgr_product_dir%': '<(output)/../qmake/product',
		'qgr_product_so_subdir%': '<(os)/<(arch)',
		'other_ldflags': [],
		'conditions': [
			['os=="android"', {
				'qgr_product_so_subdir': '<(os)/jniLibs/<(android_abi)'
			}],
			['library_output=="static_library"', {
				'other_ldflags+': [
					'-Wl,--whole-archive',
					'<(output)/obj.target/libqgr.a',
					'<(output)/obj.target/libqgr-utils.a',
					'<(output)/obj.target/libqgr-js.a',
					'<(output)/obj.target/depe/node/libnode.a',
					'-Wl,--no-whole-archive',
				],
			}],
		],
	},

	'target_defaults': {
		'conditions': [
			['more_log==1', {
				'defines': [ 'XX_MORE_LOG=1' ],
			}]
		],
		'direct_dependent_settings': {
			'include_dirs': [ '.' ],
		},
	},

	'targets': [
	{
		'target_name': 'qgr_mac_dylib',
		'type': 'none',
		'dependencies': [
			'qgr',
			'qgr-media',
			'qgr-v8',
			'qgr-js',
			'qgr-node',
		],
		'conditions': [
			# output mac shared library for "qgr.framework"
			['debug==0 and library_output=="shared_library" and OS=="mac" and project=="make"', {
				'actions': [{
					'action_name': 'mk_qgr_dylib',
					'variables': {
						'embed_bitcode%': 0,
						'use_v8_link%': 0,
						'conditions': [
							['arch in "arm arm64" and without_embed_bitcode==0', {
								'embed_bitcode': 1,
							}],
							['use_v8==0 and os=="ios"', {
								'use_v8_link': 1,
							}],
						],
					},
					'inputs': [],
					'outputs': [
						'<(output)/libqgr.dylib',
						'<(output)/libqgr-media.dylib',
						'<(output)/libqgr-v8.dylib',
						'<(output)/libqgr-js.dylib',
						'<(output)/libqgr-node.dylib',
					],
					'action': [ 
						'sh', '-c', 
						'tools/build_dylib.sh '
						'<(output) <(embed_bitcode) <(use_v8_link) '
						'<(arch_name) '
						'<(sysroot) '
						'<(version_min) '
					],
				}]
			}],
		], # conditions
	},
	{
		'target_name': 'qgr_copy_so', 
		'type': 'none',
		'dependencies': [ 'qgr', 'qgr-media', 'qgr-js', 'qgr-node' ],
		'conditions': [
			# copy libqgr.so to product directory
			['debug==0 and library_output=="shared_library" and OS!="mac"', {
				'copies': [{
					'destination': '<(qgr_product_dir)/<(qgr_product_so_subdir)',
					'files': [
						'<(output)/lib.target/libqgr.so',
						'<(output)/lib.target/libqgr-media.so',
						'<(output)/lib.target/libqgr-js.so',
						'<(output)/lib.target/libqgr-v8.so',
						'<(output)/lib.target/libqgr-node.so',
					],
				}], # copies
			}],
		], # conditions
	},
	],

	'conditions': [
		['os not in "ios osx" or project=="xcode"', {
			'includes': [
				'test/test.gypi',
				'test/test2.gypi',
				'tools/tools.gypi',
			],
		}],
	],
	
}
