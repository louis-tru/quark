{
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
}