{
	'includes': [
		'ftr/utils/utils.gypi',
		'ftr/ftr.gypi',
		'ftr-js/ftr-js.gypi',
		'trial/trial.gypi',
		'tools/default_target.gypi',
	],

	'variables': {
		'more_log%': 0,
		'ftr_product_dir%': '<(output)/../ftrp/product',
		'ftr_product_so_subdir%': '<(os)/<(arch)',
		'other_ldflags': [],
		'conditions': [
			['os=="android"', {
				'ftr_product_so_subdir': '<(os)/jniLibs/<(android_abi)'
			}],
			['library_output=="static_library"', {
				'other_ldflags+': [
					'-Wl,--whole-archive',
					'<(output)/obj.target/libftr-utils.a',
					'<(output)/obj.target/libftr.a',
					'<(output)/obj.target/libftr-js.a',
					'<(output)/obj.target/libftr-media.a',
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
		'target_name': 'libftr',
		'type': 'none',
		'dependencies': [
			'ftr',
			'ftr-js',
			'ftr-media',
			'ftr-node',
		],
	},
	{
		'target_name': 'ftr_mac_dylib',
		'type': 'none',
		'dependencies': [
			'ftr',
			'ftr-js',
			'ftr-media',
			'ftr-node',
		],
		'conditions': [
			# output mac shared library for "ftr.framework"
			['library_output=="shared_library" and OS=="mac" and project=="make"', {
				'actions': [{
					'action_name': 'mk_ftr_dylib',
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
						'<(output)/obj.target/ffmpeg/libffmpeg.a',
						'<(output)/libnghttp2.a',
						'<(output)/libcares.a',
						'<(output)/libftr-utils.a',
						'<(output)/libftr.a',
						'<(output)/libftr-js.a',
						'<(output)/libftr-media.a',
						'<(output)/libnode.a',
						'<@(lib_v8_a)',
					],
					'outputs': [
						'<(output)/libftr.dylib',
						'<(output)/libftr-js.dylib',
						'<(output)/libftr-media.dylib',
						'<(output)/libftr-node.dylib',
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
		'target_name': 'ftr_copy_so', 
		'type': 'none',
		'dependencies': [
			'ftr',
			'ftr-js',
			'ftr-media',
			'ftr-node',
		],
		'conditions': [
			# copy libftr.so to product directory
			['debug==0 and library_output=="shared_library" and OS!="mac"', {
				'copies': [{
					'destination': '<(ftr_product_dir)/<(ftr_product_so_subdir)',
					'files': [
						'<(output)/lib.target/libftr.so',
						'<(output)/lib.target/libftr-js.so',
						'<(output)/lib.target/libftr-media.so',
						'<(output)/lib.target/libftr-node.so',
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
