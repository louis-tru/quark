{
	'includes': [
		'langou/utils/utils.gypi',
		'langou/langou.gypi',
		'langou/js/js.gypi',
		'tools/default_target.gypi',
	],

	'variables': {
		'more_log%': 0,
		'langou_product_dir%': '<(output)/../qmake/product',
		'langou_product_so_subdir%': '<(os)/<(arch)',
		'other_ldflags': [],
		'conditions': [
			['os=="android"', {
				'langou_product_so_subdir': '<(os)/jniLibs/<(android_abi)'
			}],
			['library_output=="static_library"', {
				'other_ldflags+': [
					'-Wl,--whole-archive',
					'<(output)/obj.target/liblangou-utils.a',
					'<(output)/obj.target/liblangou.a',
					'<(output)/obj.target/liblangou-js.a',
					'<(output)/obj.target/liblangou-media.a',
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
		'target_name': 'liblangou',
		'type': 'none',
		'dependencies': [
			'langou',
			'langou-js',
			'langou-media',
			'langou-node',
			'langou-v8',
		],
	},
	{
		'target_name': 'langou_mac_dylib',
		'type': 'none',
		'dependencies': [
			'langou',
			'langou-js',
			'langou-media',
			'langou-node',
			'langou-v8',
		],
		'conditions': [
			# output mac shared library for "langou.framework"
			['debug==0 and library_output=="shared_library" and OS=="mac" and project=="make"', {
				'actions': [{
					'action_name': 'mk_langou_dylib',
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
						'<(output)/obj.target/FFmpeg/libFFmpeg.a',
						'<(output)/libnghttp2.a',
						'<(output)/libcares.a',
						'<(output)/liblangou-utils.a',
						'<(output)/liblangou.a',
						'<(output)/liblangou-js.a',
						'<(output)/liblangou-media.a',
						'<(output)/libnode.a',
						'<@(lib_v8_a)',
					],
					'outputs': [
						'<(output)/liblangou.dylib',
						'<(output)/liblangou-js.dylib',
						'<(output)/liblangou-media.dylib',
						'<(output)/liblangou-node.dylib',
						'<(output)/liblangou-v8.dylib',
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
		'target_name': 'langou_copy_so', 
		'type': 'none',
		'dependencies': [ 'langou', 'langou-js', 'langou-media', 'langou-node', 'langou-v8' ],
		'conditions': [
			# copy liblangou.so to product directory
			['debug==0 and library_output=="shared_library" and OS!="mac"', {
				'copies': [{
					'destination': '<(langou_product_dir)/<(langou_product_so_subdir)',
					'files': [
						'<(output)/lib.target/liblangou.so',
						'<(output)/lib.target/liblangou-js.so',
						'<(output)/lib.target/liblangou-media.so',
						'<(output)/lib.target/liblangou-node.so',
						'<(output)/lib.target/liblangou-v8.so',
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
