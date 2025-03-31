{
	'includes': [
		'src/util/util.gypi',
		'src/src.gypi',
		'trial/trial.gypi',
		'tools/default_target.gypi',
	],

	'target_defaults': {
		'direct_dependent_settings': {
			'include_dirs': [ '.' ],
		},
	},

	'variables': {
		'product_dir%': '<(DEPTH)/out/qkmake/product',
		'product_so_subdir%': '<(os)/<(arch)',
		'conditions': [
			['os=="android"', {
				'product_so_subdir': '<(os)/jniLibs/<(android_abi)'
			}],
		],
	},

	############################################
	# targets output
	############################################

	'targets': [{
		'target_name': 'libquark',
		'product_name': 'quark',
		'type': 'none',
		'dependencies': [
			'quark',
			'quark-media',
			'quark-js',
		],
		'conditions': [
			# output mac shared library for "quark.framework"
			['os in "mac ios" and project=="make"', {
				'actions': [{
					'action_name': 'mk_quark_dylib',
					'variables': {
						'embed_bitcode%': 0,
						'lib_v8_a%': [],
						'conditions': [
							['arch in "arm arm64" and without_embed_bitcode==0', {
								'embed_bitcode': 1,
							}],
							['use_v8==1', {
								'lib_v8_a': [
									'<(output)/libv8_base.a', 
									'<(output)/libv8_libbase.a',
									'<(output)/libv8_libsampler.a',
									'<(output)/libv8_builtins_setup.a',
									'<(output)/libv8_nosnapshot.a',
									'<(output)/libv8_libplatform.a',
								],
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
						'<(output)/libquark-utils.a',
						'<(output)/libquark.a',
						'<(output)/libquark-media.a',
						'<(output)/libquark-js.a',
						'<@(lib_v8_a)',
					],
					'outputs': [
						'<(output)/libquark.dylib',
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
			['os not in "mac ios"', {
				'type': 'shared_library',
				'ldflags': [ '-Wl,--version-script,<(source)/tools/v_small.ver' ],
				'copies': [{
					'destination': '<(product_dir)/<(product_so_subdir)',
					'files': [ 'out/<(output_name)/obj.target/libquark.so' ],
				}], # copy libquark.so to product directory
			}],
		], # conditions
	}],

	# output executed binrary
	'conditions': [
		['use_js==1', {
			'includes': ['src/js/js.gypi'],
		},{
			'targets': [{
				'target_name': 'quark-js',
				'type': 'none',
			}],
		}],
		['os not in "mac ios" or project=="xcode"', {
			'includes': [ 'test/test.gypi' ],
		}],
		['use_js==1', {
			'targets+': [
			{
				'target_name': 'quark-exec',
				'product_name': 'quark', # output name quark
				'type': 'executable',
				'dependencies': [
					'libquark'
				],
				'conditions': [['os=="android" and host_os=="mac"', {
					# Unknown reason, Only cross-compiling on the Mac host of need use these flags
					'ldflags': [
						'-lGLESv3','-lEGL','-lz',
						'-landroid','-llog','-latomic',
						'-lm','-lOpenSLES','-lmediandk',
					],
				}]],
				'sources': [
					'src/js/main.cc',
				], # sources
			}],
		}]
	],
}
