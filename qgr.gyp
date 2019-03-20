{
	'includes': [
		'qgr/utils/utils.gypi',
		'qgr/qgr.gypi',
		'qgr/js/js.gypi',
		'tools/default_target.gypi',
	],

	'variables': {
		'qgr_product_dir%': '<(output)/../qgr-tools/product',
		'qgr_product_so_subdir%': '<(os)/<(arch)',
		'more_log%': 0,
		'conditions': [
			['os=="android"', {
				'qgr_product_so_subdir': '<(os)/jniLibs/<(android_abi)'
			}],
		],
	},

	'target_defaults': {
		'conditions': [
			['more_log==1', {
				'defines': [ 'XX_MORE_LOG=1' ],
			}]
		],
	},

	'targets': [{
		'target_name': 'qgr-lib',
		'product_name': 'qgr',
		'type': 'none',
		'dependencies': [
			'qgr-utils', 
			'qgr', 
			'qgr-js', 
		],
		'conditions': [
			['library_output=="shared_library" and OS not in "mac"', {
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
				'libs/qgr',
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
		'target_name': 'qgr_copy_so', 
		'type': 'none',
		'dependencies': [ 'qgr-lib' ],
		
		'conditions': [
			# copy libqgr.so to product directory
			['debug==0 and library_output=="shared_library" and OS not in "mac"', {
				'copies': [{
					'destination': '<(qgr_product_dir)/<(qgr_product_so_subdir)',
					'files': [
						'<(output)/lib.target/libqgr.so',
					],
				}], # copies
			}],
			# output mac shared library for "qgr.framework"
			['debug==0 and library_output=="shared_library" and OS in "mac" and project=="make"', {
				'actions': [{
					'action_name': 'qgr_apple_dylib',
					'variables': {
						'embed_bitcode': '',
						'inputs_node': '',
						'LinkFileList_node': '',
						'link_node': '',
						'conditions': [
							['arch in "arm arm64" and without_embed_bitcode==0', {
								'embed_bitcode': '-fembed-bitcode',
							}],
							['use_v8==0 and os=="ios"', {
								'v8libs': [ '<(output)/libv8-link.a', ],
								'v8libs_l': '-lv8-link',
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
								'v8libs_l': '-lv8_base '
														'-lv8_libbase '
														'-lv8_libsampler '
														'-lv8_builtins_setup '
														'-lv8_builtins_generators '
														'-lv8_nosnapshot '
														'-lv8_libplatform ',
							}],
							['node_enable==1', {
								'inputs_node': '<(output)/libnode.a',
								'LinkFileList_node': 'obj.target/node/depe/node',
								'link_node': '-lnode',
							}],
						],
					},
					'inputs': [
						'<(output)/libqgr-utils.a', 
						'<(output)/libqgr.a', 
						'<(output)/libqgr-js.a',
						'<(output)/libminizip.a',
						'<(output)/libopenssl.a',
						'<(output)/libuv.a',
						'<(output)/libhttp_parser.a',
						'<(output)/libreachability.a',
						'<(output)/libtess2.a',
						'<(output)/libft2.a',
						'<(output)/libtinyxml2.a',
						'<(output)/obj.target/depe/ffmpeg/libffmpeg.a',
						'<(inputs_node)',
						'<(output)/libnghttp2.a',
						'<(output)/libcares.a',
						'<@(v8libs)',
					],
					'outputs': [
						'<(output)/libqgr.dylib',
					],
					'action': [ 'sh', '-c', 
						'cd <(output);'
						'find obj.target/qgr-utils ' 
						'obj.target/qgr '
						'obj.target/qgr-js '
						'<(LinkFileList_node) '
						'-name *.o > qgr.LinkFileList;'
						'clang++ '
						'-arch <(arch_name) -dynamiclib '
						'-isysroot <(sysroot) '
						'-L<(output) '
						'-L<(output)/obj.target/ffmpeg '
						'-L<(sysroot)/System/Library/Frameworks '
						'-stdlib=libc++ '
						'-filelist <(output)/qgr.LinkFileList '
						'-o <@(_outputs) '
						'-single_module '
						'-install_name @rpath/qgr.framework/qgr '
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
						'<(link_node) '
						'-lnghttp2 '
						'-lcares '
						'<(v8libs_l) '
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
				'test/test2.gypi',
				'tools/tools.gypi',
			],
		}],
	],
	
}
