{
	'variables': {
		'cplusplus_exceptions%': 1,
		'cplusplus_rtti%': 1,
		'library_output%': 'static_library',
		'output_type%': 'static_library',
		'conditions': [
			['library_output=="shared_library" and os not in "mac ios"',{
				'output_type': 'shared_library',
			}],
		],
	},

	'target_defaults': {
		'conditions': [
			['library_output=="shared_library"', { 
				'defines': [ 'Qk_BUILDING_SHARED' ],
			}],
			# c++ exceptions
			['cplusplus_exceptions==1', {
				'xcode_settings': {
					'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',   # -fexceptions
				},
				'cflags_cc!': [ '-fno-exceptions' ],
				'cflags_cc': [ '-fexceptions' ],
			},{
				'xcode_settings': {
					'GCC_ENABLE_CPP_EXCEPTIONS': 'NO',   # -fno-exceptions
				},
				'cflags_cc!': [ '-fexceptions' ],
				'cflags_cc': [ '-fno-exceptions' ],
			}],
			# c++ rtti
			['cplusplus_rtti==1', {
				'xcode_settings': {
					'GCC_ENABLE_CPP_RTTI': 'YES',         # -frtti / -fno-rtti
				},
				'cflags_cc!': [ '-fno-rtti' ],
				'cflags_cc': [ '-frtti', ],
			}, {
				'xcode_settings': {
					'GCC_ENABLE_CPP_RTTI': 'NO',          # -frtti / -fno-rtti
				},
				'cflags_cc!': [ '-frtti' ],
				'cflags_cc': [ '-fno-rtti', ],
			}],
		],
	},
}