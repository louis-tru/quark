{
	'includes': [
		'flare/util/util.gypi',
		'flare/flare.gypi',
		# 'flare/js/js.gypi',
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
		'flare_product_dir%': '<(output)/../fproj/product',
		'flare_product_so_subdir%': '<(os)/<(arch)',
		'other_ldflags': [],
		'conditions': [
			['os=="android"', {
				'flare_product_so_subdir': '<(os)/jniLibs/<(android_abi)'
			}],
			['library_output=="static_library"', {
				'other_ldflags+': [
					'-Wl,--whole-archive',
					'<(output)/obj.target/libflare-utils.a',
					'<(output)/obj.target/libflare.a',
					'<(output)/obj.target/libflare-media.a',
					# '<(output)/obj.target/libflare-js.a',
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
		'target_name': 'libflare',
		'type': 'none',
		'dependencies': [
			'flare',
			'flare-media',
			# 'flare-js',
			# 'flare-node',
		],
		'conditions': [
			# output mac shared library for "flare.framework"
			['library_output=="shared_library" and OS=="mac" and project=="make"', {
				'actions': [{
					'action_name': 'mk_flare_dylib',
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
						'<(output)/libflare-utils.a',
						'<(output)/libflare.a',
						'<(output)/libflare-media.a',
						# '<(output)/libflare-js.a',
						# '<(output)/libnode.a',
						'<@(lib_v8_a)',
					],
					'outputs': [
						'<(output)/libflare.dylib',
						# '<(output)/libflare-js.dylib',
						# '<(output)/libflare-media.dylib',
						# '<(output)/libflare-node.dylib',
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
			# output not mac shared library for "flare.so"
			['library_output=="shared_library" and OS!="mac"', {
				'product_name': 'flare',
				'type': 'shared_library',
				# 'product_prefix': 'flare',
				# 'product_extension': 'so',
				'copies': [{
					'destination': '<(flare_product_dir)/<(flare_product_so_subdir)',
					'files': [
						'<(output)/lib.target/libflare.so',
						# '<(output)/lib.target/libflare-js.so',
						# '<(output)/lib.target/libflare-media.so',
						# '<(output)/lib.target/libflare-node.so',
					],
				}], # copy libflare.so to product directory
			}],
		], # conditions
	}
	],

}
