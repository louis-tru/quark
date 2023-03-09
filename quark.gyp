{
	'includes': [
		'src/util/util.gypi',
		'src/src.gypi',
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
		'quark_product_dir%': '<(output)/../qkmake/product',
		'quark_product_so_subdir%': '<(os)/<(arch)',
		'other_ldflags': [],
		'conditions': [
			['os=="android"', {
				'quark_product_so_subdir': '<(os)/jniLibs/<(android_abi)'
			}],
			['library_output=="static_library"', {
				'other_ldflags+': [
					'-Wl,--whole-archive',
					'<(output)/obj.target/libquark-utils.a',
					'<(output)/obj.target/libquark.a',
					'<(output)/obj.target/libquark-media.a',
					# '<(output)/obj.target/libquark-js.a',
					'-Wl,--no-whole-archive',
				],
			}],
		],
	},

	'target_defaults': {
		'conditions': [
			['more_log==1', {
				'defines': [ 'Qk_MORE_LOG=1' ],
			}],
		],
		'direct_dependent_settings': {
			'include_dirs': [ '.' ],
		},
	},

	'targets': [
	{
		'target_name': 'libquark',
		'type': 'none',
		'dependencies': [
			'quark',
			'quark-media',
			# 'quark-js',
		],
		'conditions': [
			# output mac shared library for "quark.framework"
			['library_output=="shared_library" and OS=="mac" and project=="make"', {
				'actions': [{
					'action_name': 'mk_quark_dylib',
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
						'<(output)/libfreetype.a',
						'<(output)/libtinyxml2.a',
						'<(output)/obj.target/ffmpeg/libffmpeg.a',
						# '<(output)/libnghttp2.a',
						# '<(output)/libcares.a',
						'<(output)/libquark-utils.a',
						'<(output)/libquark.a',
						'<(output)/libquark-media.a',
						# '<(output)/libquark-js.a',
						'<@(lib_v8_a)',
					],
					'outputs': [
						'<(output)/libquark.dylib',
						# '<(output)/libquark-media.dylib',
						# '<(output)/libquark-js.dylib',
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
			# output not mac shared library for "quark.so"
			['library_output=="shared_library" and OS!="mac"', {
				'product_name': 'quark',
				'type': 'shared_library',
				# 'product_prefix': 'quark',
				# 'product_extension': 'so',
				'copies': [{
					'destination': '<(quark_product_dir)/<(quark_product_so_subdir)',
					'files': [
						'<(output)/lib.target/libquark.so',
						# '<(output)/lib.target/libquark-media.so',
						# '<(output)/lib.target/libquark-js.so',
					],
				}], # copy libquark.so to product directory
			}],
		], # conditions
	}
	],

}
