{  
	'includes': [
		'ngui/base/base.gypi',
		'ngui/ngui.gypi',
		'ngui/js/js.gypi',
	],

	'variables': {
		'cplusplus_exceptions%': 1,
		'cplusplus_rtti%': 1,
		'output_library%': 'static_library',
		'ngui_product_dir%': '<(output)/../ngui-tools/product',
		'ngui_product_so_subdir%': '<(os)/<(arch)',
		'conditions': [
			['os=="android"', {
				'ngui_product_so_subdir': '<(os)/jniLibs/<(android_abi)'
			}],
		],
	},
	
	'target_defaults': {
		'conditions': [
			['output_library=="shared_library"', { 
				'defines': [ 'XX_BUILDING_SHARED' ],
			}],
			# c++ exceptions
			['cplusplus_exceptions==1', {
				'xcode_settings': {
					'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',   # -fexceptions
				},
				'cflags_cc': [ '-fexceptions' ], 
			},{
				'xcode_settings': {
					'GCC_ENABLE_CPP_EXCEPTIONS': 'NO',   # -fno-exceptions
				},
				'cflags_cc': [ '-fno-exceptions' ], 
			}],
			# c++ rtti
			['cplusplus_rtti==1', {
				'xcode_settings': {
					'GCC_ENABLE_CPP_RTTI': 'YES',         # -frtti / -fno-rtti
				},
				'cflags_cc': [ '-frtti', ], 
			}, {
				'xcode_settings': {
					'GCC_ENABLE_CPP_RTTI': 'NO',          # -frtti / -fno-rtti
				},
				'cflags_cc': [ '-fno-rtti', ], 
			}],
		],
	},
	
	'targets': [{
		'target_name': 'ngui-lib',
		'product_name': 'ngui',
		'type': 'none',
		'dependencies': [ 
			'ngui-base', 
			'ngui-gui', 
			'ngui-js', 
		],
		'conditions': [
			['output_library=="shared_library" and OS not in "mac"', {
				'type': 'shared_library',
				'direct_dependent_settings': {
					'defines': [ 'XX_USIXX_SHARED' ],
				},
			}],
		],
		'direct_dependent_settings': {
			'include_dirs': [ '.' ],
			'xcode_settings': {
			 'OTHER_LDFLAGS': '-all_load',
			},
			'mac_bundle_resources': [
				'node_modules/ngui',
			],
			'conditions': [
				['cplusplus_exceptions==1', {
					'xcode_settings': {
						'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
					},
					'cflags_cc': [ '-fexceptions' ], 
				}],
				['cplusplus_rtti==1', {
					'xcode_settings': { 
						'GCC_ENABLE_CPP_RTTI': 'YES', 
					},
					'cflags_cc': [ '-frtti', ], 
				}],
			],
		}, # direct_dependent_settings
	}, {
		'target_name': 'ngui_copy_so', 
		'type': 'none',
		'dependencies': [ 'ngui-lib' ],
		
		'conditions': [
			# copy libngui.so to product directory
			['debug==0 and output_library=="shared_library" and OS not in "mac"', {
				'copies': [{
					'destination': '<(ngui_product_dir)/<(ngui_product_so_subdir)',
					'files': [
						'<(output)/lib.target/libngui.so',
					],
				}], # copies
			}],
			# output mac shared library for "ngui.framework"
			['debug==0 and output_library=="shared_library" \
				and OS in "mac" and project=="make"', {
				'actions': [{
					'action_name': 'ngui_apple_dylib',
					'variables': {
						'embed_bitcode': '',
						'conditions': [
							['arch in "arm arm64" and without_embed_bitcode==0', {
								'embed_bitcode': '-fembed-bitcode',
							}],
							['use_v8==0 and os=="ios"', {
								'v8libs': [ '<(output)/libv8-link.a', ],
								'l_v8libs': '-lv8-link',
							}, {
								'v8libs': [ 
									'<(output)/libv8_base.a',
									'<(output)/libv8_libbase.a',
									'<(output)/libv8_libsampler.a',
									'<(output)/libv8_builtins_setup.a',
									'<(output)/libv8_builtins_generators.a',
									'<(output)/libv8_nosnapshot.a',
									'<(output)/libv8_libplatform.a',
								],
								'l_v8libs': '-lv8_base '
														'-lv8_libbase '
														'-lv8_libsampler '
														'-lv8_builtins_setup '
														'-lv8_builtins_generators '
														'-lv8_nosnapshot '
														'-lv8_libplatform ',
							}],
						],
					},
					'inputs': [
						'<(output)/libngui-base.a', 
						'<(output)/libngui-gui.a', 
						'<(output)/libngui-js.a',
						'<(output)/libminizip.a',
						'<(output)/libopenssl.a',
						'<(output)/libuv.a',
						'<(output)/libhttp_parser.a',
						'<(output)/libreachability.a',
						'<(output)/libtess2.a',
						'<(output)/libft2.a',
						'<(output)/libtinyxml2.a',
						'<(output)/obj.target/ffmpeg/libffmpeg.a',
						'<(output)/libnode.a',
						'<(output)/libnghttp2.a',
						'<(output)/libcares.a',
						'<@(v8libs)',
					],
					'outputs': [
						'<(output)/libngui.dylib',
					],
					'action': [ 'sh', '-c', 
						'cd <(output);'
						'find obj.target/ngui-base ' 
						'obj.target/ngui-gui '
						'obj.target/ngui-js '
						'obj.target/node/node -name *.o > ngui.LinkFileList;'
						'clang++ '
						'-arch <(arch_name) -dynamiclib '
						'-isysroot <(sysroot) '
						'-L<(output) '
						'-L<(output)/obj.target/ffmpeg '
						'-L<(sysroot)/System/Library/Frameworks '
						'-stdlib=libc++ '
						'-filelist <(output)/ngui.LinkFileList '
						'-o <@(_outputs) '
						'-single_module '
						'-install_name @rpath/ngui.framework/ngui '
						'-Xlinker -rpath -Xlinker @executable_path/Frameworks '
						'-Xlinker -rpath -Xlinker @loader_path/Frameworks '
						'-miphoneos-version-min=<(version_min) '
						'<(embed_bitcode) '
						'-dead_strip '
						'-fobjc-arc '
						'-fobjc-link-runtime  '
						# Link static library
						'-lminizip '
						'-lopenssl '
						'-luv '
						'-lhttp_parser '
						'-lreachability '
						'-ltess2 '
						'-lft2 '
						'-ltinyxml2 '
						'-lffmpeg '
						'-lnode '
						'-lnghttp2 '
						'-lcares '
						'<(l_v8libs) '
						# Link system library
						'-framework Foundation '
						'-framework SystemConfiguration '
						'-framework OpenGLES '
						'-framework CoreGraphics '
						'-framework UIKit '
						'-framework QuartzCore '
						'-framework AudioToolbox '
						'-framework CoreVideo '
						'-framework VideoToolbox '
						'-framework CoreMedia '
						'-framework JavaScriptCore '
						'-framework MessageUI '
						'-liconv '
						'-lbz2 '
						'-lsqlite3 '
						'-lz'
					],
				}]
			}],
		], # conditions
	}],
	
	'conditions': [
		['os not in "ios osx" or project=="xcode"', {
			'includes': [
				'test/test.gypi',
				'tools/tools.gypi',
			],
		}],
	],
}
