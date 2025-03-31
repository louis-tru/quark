{
	'variables': {
		'cplusplus_exceptions%': 1,
		'cplusplus_rtti%': 1,
	},

	'target_defaults': {
		'conditions': [
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