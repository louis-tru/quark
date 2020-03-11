{
	'includes': [
		'nxkit/nxkit.gypi',
		'ngui/ngui.gypi',
		'ngui/js/js.gypi',
		'tools/default_target.gypi',
	],

	'variables': {
		'more_log%': 0,
		'ngui_product_dir%': '<(output)/../nxmake/product',
		'ngui_product_so_subdir%': '<(os)/<(arch)',
		'other_ldflags': [],
		'conditions': [
			['os=="android"', {
				'ngui_product_so_subdir': '<(os)/jniLibs/<(android_abi)'
			}],
			['library_output=="static_library"', {
				'other_ldflags+': [
					'-Wl,--whole-archive',
					'<(output)/obj.target/libnxkit.a',
					'<(output)/obj.target/libngui.a',
					'<(output)/obj.target/libngui-js.a',
					'<(output)/obj.target/libngui-media.a',
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
		'target_name': 'libngui',
		'type': 'none',
		'dependencies': [
			'nxkit',
			'ngui',
			'ngui-js',
			'ngui-media',
			'ngui-node',
			'ngui-v8',
		],
	},
	{
		'target_name': 'ngui_mac_dylib',
		'type': 'none',
		'dependencies': [
			'nxkit',
			'ngui',
			'ngui-js',
			'ngui-media',
			'ngui-node',
			'ngui-v8',
		],
		'conditions': [
			# output mac shared library for "ngui.framework"
			['debug==0 and library_output=="shared_library" and OS=="mac" and project=="make"', {
				'actions': [{
					'action_name': 'mk_ngui_dylib',
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
						'<(output)/libnxkit.a',
						'<(output)/libngui.a',
						'<(output)/libngui-js.a',
						'<(output)/libngui-media.a',
						'<(output)/libnode.a',
						'<@(lib_v8_a)',
					],
					'outputs': [
						'<(output)/libnxkit.dylib',
						'<(output)/libngui.dylib',
						'<(output)/libngui-js.dylib',
						'<(output)/libngui-media.dylib',
						'<(output)/libngui-node.dylib',
						'<(output)/libngui-v8.dylib',
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
		'target_name': 'ngui_copy_so', 
		'type': 'none',
		'dependencies': [
			'nxkit',
			'ngui',
			'ngui-js',
			'ngui-media',
			'ngui-node',
			'ngui-v8',
		],
		'conditions': [
			# copy libngui.so to product directory
			['debug==0 and library_output=="shared_library" and OS!="mac"', {
				'copies': [{
					'destination': '<(ngui_product_dir)/<(ngui_product_so_subdir)',
					'files': [
						'<(output)/lib.target/libnxkit.so',
						'<(output)/lib.target/libngui.so',
						'<(output)/lib.target/libngui-js.so',
						'<(output)/lib.target/libngui-media.so',
						'<(output)/lib.target/libngui-node.so',
						'<(output)/lib.target/libngui-v8.so',
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
