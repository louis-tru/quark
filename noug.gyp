{
	'includes': [
		'src/util/util.gypi',
		'src/noug.gypi',
		# 'noug/js/js.gypi',
		'trial/trial.gypi',
		'tools/default_target.gypi',
	],

	'conditions': [
		['os not in "ios osx" or project=="xcode"', {
			'includes': [
				'test/test.gypi',
			],
		}],
	],

	'variables': {
		'more_log%': 0,
		'noug_product_dir%': '<(output)/../noproj/product',
		'noug_product_so_subdir%': '<(os)/<(arch)',
		'other_ldflags': [],
		'conditions': [
			['os=="android"', {
				'noug_product_so_subdir': '<(os)/jniLibs/<(android_abi)'
			}],
			['library_output=="static_library"', {
				'other_ldflags+': [
					'-Wl,--whole-archive',
					'<(output)/obj.target/libnoug-utils.a',
					'<(output)/obj.target/libnoug.a',
					'<(output)/obj.target/libnoug-media.a',
					# '<(output)/obj.target/libnoug-js.a',
					# '<(output)/obj.target/deps/node/libnode.a',
					'-Wl,--no-whole-archive',
				],
			}],
		],
	},

	'target_defaults': {
		'conditions': [
			['more_log==1', {
				'defines': [ 'FX_MORE_LOG=1' ],
			}],
		],
		'direct_dependent_settings': {
			'include_dirs': [ '.' ],
		},
	},

	'targets': [
	{
		'target_name': 'libnoug',
		'type': 'none',
		'dependencies': [
			'noug',
			'noug-media',
			# 'noug-js',
			# 'noug-node',
		],
		'conditions': [
			# output mac shared library for "noug.framework"
			['library_output=="shared_library" and OS=="mac" and project=="make"', {
				'actions': [{
					'action_name': 'mk_noug_dylib',
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
						'<(output)/libnoug-utils.a',
						'<(output)/libnoug.a',
						'<(output)/libnoug-media.a',
						# '<(output)/libnoug-js.a',
						# '<(output)/libnode.a',
						'<@(lib_v8_a)',
					],
					'outputs': [
						'<(output)/libnoug.dylib',
						# '<(output)/libnoug-js.dylib',
						# '<(output)/libnoug-media.dylib',
						# '<(output)/libnoug-node.dylib',
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
			# output not mac shared library for "noug.so"
			['library_output=="shared_library" and OS!="mac"', {
				'product_name': 'noug',
				'type': 'shared_library',
				# 'product_prefix': 'noug',
				# 'product_extension': 'so',
				'copies': [{
					'destination': '<(noug_product_dir)/<(noug_product_so_subdir)',
					'files': [
						'<(output)/lib.target/libnoug.so',
						# '<(output)/lib.target/libnoug-js.so',
						# '<(output)/lib.target/libnoug-media.so',
						# '<(output)/lib.target/libnoug-node.so',
					],
				}], # copy libnoug.so to product directory
			}],
		], # conditions
	}
	],

}
