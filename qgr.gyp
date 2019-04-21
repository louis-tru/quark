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
					'<(output)/obj.target/libqgr-media.a',
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
			}],
		],
		'direct_dependent_settings': {
			'include_dirs': [ '.' ],
		},
	},

	'targets': [
	{
		'target_name': 'libqgr',
		'type': 'none',
		'dependencies': [
			'qgr',
			'qgr-media',
			'qgr-v8',
			'qgr-js',
			'qgr-node',
		],
	},
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
						'lib_v8_a%': [ 
							'<(output)/libv8_base.a', 
							'<(output)/libv8_libbase.a',
							'<(output)/libv8_libsampler.a',
							'<(output)/libv8_builtins_setup.a',
							'<(output)/libv8_nosnapshot.a',
							'<(output)/libv8_libplatform.a',
						],
						'conditions': [
							['arch in "arm arm64" and without_embed_bitcode==0', {
								'embed_bitcode': 1,
							}],
							['use_v8==0 and os=="ios"', {
								'use_v8_link': 1,
								'lib_v8_a': [ '<(output)/libv8-link.a' ],
							}],
						],
					},
					'inputs': [
						'tools/build_dylib.sh',
						'<(output)/libuv.a',
						'<(output)/libopenssl.a',
						'<(output)/libhttp_parser.a',
						'<(output)/libminizip.a',
						'<(output)/libreachability.a',
						'<(output)/libtess2.a',
						'<(output)/libft2.a',
						'<(output)/libtinyxml2.a',
						'<(output)/libqgr-utils.a',
						'<(output)/libqgr.a',
						'<(output)/obj.target/FFmpeg/libFFmpeg.a',
						'<(output)/libqgr-media.a',
						'<@(lib_v8_a)',
						'<(output)/libqgr-js.a',
						'<(output)/libnghttp2.a',
						'<(output)/libcares.a',
						'<(output)/libnode.a',
					],
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
					# 'process_outputs_as_sources': 1,
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
			],
		}],
	],
	
}
