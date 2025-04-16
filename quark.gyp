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
					'inputs': [
						'tools/build_dylib.sh',
						'<(output)/libbptree.a',
						'<(output)/libfreetype.a',
						'<(output)/libgif.a',
						'<(output)/libhttp_parser.a',
						'<(output)/libminizip.a',
						'<(output)/libopenssl.a',
						'<(output)/libquark-media.a',
						'<(output)/libquark-util.a',
						'<(output)/libquark.a',
						'<(output)/libreachability.a',
						'<(output)/libtess2.a',
						'<(output)/libuv.a',
						'<(output)/obj.target/ffmpeg/libffmpeg.a',
					],
					'outputs': [
						'<(output)/libquark.dylib',
					],
					'action': [
						'sh', '-c',
						'tools/build_dylib.sh '
						'<(output) '
						'<(os) '
						'<(arch_name) '
						'<(sysroot) '
						'<(version_min) '
						'<(without_embed_bitcode) '
						'<(use_js) '
						'<(use_v8) '
						'<(emulator) '
					],
					'conditions': [
						['use_js==1', {
							'inputs+': [
								'<(output)/libquark-js.a',
							],
						}],
						['use_js==1 and use_v8==1', {
							'inputs+': [
								'<(output)/libv8_initializers.a',
								'<(output)/libv8_libbase.a',
								'<(output)/libv8_libplatform.a',
								'<(output)/libv8_libsampler.a',
								'<(output)/libv8_snapshot.a',
								'<(output)/libv8_base_without_compiler.a',
								'<(output)/libv8_compiler.a',
							],
						}],
					],
					# 'process_outputs_as_sources': 1,
				}]
			}],
			['os not in "mac ios"', {
				'type': 'shared_library',
				'ldflags': [ '-Wl,--version-script,<(source)/tools/v_small.ver' ],
				'copies': [{
					'destination': '<(product_dir)/<(product_so_subdir)',
					'files': [
						'out/<(output_name)/obj.target/libquark.so',
					],
				}], # copy libquark.so to product directory
			}]
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
			'targets+': [{
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
				'xcode_settings': {
					'OTHER_LDFLAGS': ['-arch <(arch_name)'],
				},
			}],
		}]
	],
}
