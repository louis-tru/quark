{
	'variables': {
		'OS%': 'mac',
		'os%': 'mac',
	},
	'target_defaults': {
		'default_configuration': 'Release',
		'configurations': {
			'Debug': {
				'defines': [ 'DEBUG=1', '_DEBUG' ],
				'cflags': [ '-g', '-O0' ],
				'xcode_settings': {
					'GCC_OPTIMIZATION_LEVEL': '0',
					'ONLY_ACTIVE_ARCH': 'YES',      # Build Active Architecture Only
				},
			},
			'Release': {
				'defines': [ 'NDEBUG' ],
				'cflags': [
					'-Os', 
					'-ffunction-sections', 
					'-fdata-sections', 
					'-fvisibility=hidden',
					'-fomit-frame-pointer',
				],
				'cflags_cc': [
					'-fvisibility-inlines-hidden',
				],
				'ldflags': [ '-s' ],
				'xcode_settings': {
					'GCC_OPTIMIZATION_LEVEL': 's',  # -Os
					'GCC_STRICT_ALIASING': 'YES',
					'ONLY_ACTIVE_ARCH': 'YES',
					'GCC_SYMBOLS_PRIVATE_EXTERN': 'YES',  # -fvisibility=hidden
				},
				'conditions': [
					['os=="android"', {
						'cflags!': [ '-Os' ],
						'cflags': [ '-O2' ],
					}],
				],
			},
		},
		'type': 'static_library',
		'cflags!': ['-Werror'],
		'xcode_settings': {
			'CLANG_CXX_LANGUAGE_STANDARD': 'c++14',   # -std=c++0x
			'CLANG_CXX_LIBRARY': 'libc++',            # c++11 libc support
			'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',       # -fexceptions
			'GCC_ENABLE_CPP_RTTI': 'YES',             # -frtti / -fno-rtti
		},
		'cflags_cc': [ '-std=c++14', '-fexceptions', '-frtti', ],
		'defines': [],
		'conditions': [
			['os=="android"', {
				'cflags!': [ '-Wno-old-style-declaration' ],
				'cflags': [ '-fPIC', '-pthread' ],
				'ldflags': [
					'-Wl,--gc-sections',  # Discard Unused Functions with gc-sections
					'-pthread',
					# '-shared',
					# '-rdynamic',
					'-stdlib=libc++'
				],
				'link_settings': {
					'libraries': [
						'-llog',
						'-landroid',
						'-lquark',
					],
					'library_dirs': [
						'<(DEPTH)/out/usr/android/jniLibs/${ANDROID_ABI}',
					],
				},
				'include_dirs': [ '<(DEPTH)/out/usr/include' ],
			}],
			['os=="linux"', {
				# 'defines': [ '__STDC_LIMIT_MACROS' ],
				'cflags': [
					# '-fPIC',
					'-Wall',
					'-Wextra',
					'-Wno-unused-parameter',
					'-Wno-unused-but-set-parameter',
					'-Wno-unused-variable',
					'-Wno-unused-but-set-variable',
					'-Wno-unused-result',
					'-Wno-unused-function',
					'-Wno-deprecated',
					'-Wno-missing-field-initializers',
				],
				'cflags_cc': [ '-Wno-reorder' ],
				'ldflags': [
					'-pthread',
					#'-rdynamic',
				],
				'link_settings': {
					'libraries': [ '-lquark' ],
					'library_dirs': [
						'<(DEPTH)/out/usr/linux/${LINUX_ARCH}',
					],
				},
				'include_dirs': [ '<(DEPTH)/out/usr/include' ],
			}],
			['os=="ios"', {
				'xcode_settings': {
					'SYMROOT': '<(DEPTH)/out/xcodebuild/<(os)',
					'ALWAYS_SEARCH_USER_PATHS': 'NO',
					'GCC_TREAT_WARNINGS_AS_ERRORS': 'NO',
					'SDKROOT': 'iphoneos',
					'TARGETED_DEVICE_FAMILY': '1,2',
					'IPHONEOS_DEPLOYMENT_TARGET': '10.0',
					'USE_HEADERMAP': 'NO',
					'ARCHS': [ '$(ARCHS_STANDARD)' ],   # 'ARCHS': [ '$(ARCHS_STANDARD_32_BIT)' ],
					'SKIP_INSTALL': 'YES',
					'ENABLE_BITCODE': 'NO',
					'DEBUG_INFORMATION_FORMAT': 'dwarf', # dwarf-with-dsym
					'LD_RUNPATH_SEARCH_PATHS': [
						'$(inherited)', 
						'@executable_path/Frameworks'
					],
					'CLANG_ENABLE_OBJC_ARC': 'YES',
					# 'VALID_ARCHS': ['arm64'], # Xcode 12 has abandoned this option
				},
				'cflags_cc': [ '-stdlib=libc++' ],
				'link_settings': {
					'libraries': [
						'$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
						'$(SDKROOT)/System/Library/Frameworks/UIKit.framework',
						'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/quark.framework',
					],
				},
				'mac_framework_dirs': [
					'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)',
				],
				'mac_bundle_frameworks': [
					'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/quark.framework',
				],
				# 'include_dirs': [ '<(DEPTH)/out/libs/include' ],
			}],
			['os=="mac"', {
				'xcode_settings': {
					'SYMROOT': '<(DEPTH)/out/xcodebuild/<(os)',
					'ALWAYS_SEARCH_USER_PATHS': 'NO',
					'GCC_TREAT_WARNINGS_AS_ERRORS': 'NO',
					'SDKROOT': 'macosx',
					'GCC_CW_ASM_SYNTAX': 'NO',                # No -fasm-blocks
					'GCC_DYNAMIC_NO_PIC': 'NO',               # No -mdynamic-no-pic
																										# (Equivalent to -fPIC)
					'GCC_ENABLE_PASCAL_STRINGS': 'NO',        # No -mpascal-strings
					'GCC_THREADSAFE_STATICS': 'NO',           # -fno-threadsafe-statics
					'PREBINDING': 'NO',                       # No -Wl,-prebind
					'MACOSX_DEPLOYMENT_TARGET': '10.8',       # -mmacosx-version-min=10.8
					'USE_HEADERMAP': 'NO',
					'ARCHS': ['$(ARCHS_STANDARD)'],           # 'ARCHS': 'x86_64',
					'SKIP_INSTALL': 'YES',
					'ENABLE_BITCODE': 'NO',
					'DEBUG_INFORMATION_FORMAT': 'dwarf',      # dwarf-with-dsym
					'LD_RUNPATH_SEARCH_PATHS': [
						'$(inherited)', 
						'@executable_path/Frameworks'
					],
					'CLANG_ENABLE_OBJC_ARC': 'YES',
				},
				'cflags_cc': [ '-stdlib=libc++' ], 
				'link_settings': {
					'libraries': [
						'$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
						'$(SDKROOT)/System/Library/Frameworks/AppKit.framework',
						'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/quark.framework',
					],
				},
				'mac_framework_dirs': [
					'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)',
				],
				'mac_bundle_frameworks': [
					'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/quark.framework',
				],
			}],
		],
	},

	'targets': [{
		'target_name': 'libquark',
		'type': 'none',
	}],

	'conditions': [
		['os=="ios"', {
			'xcode_settings': {
				'SYMROOT': '<(DEPTH)/out/xcodebuild/<(os)',
				'SDKROOT': 'iphoneos',
			},
		}, 
		'os=="mac"', {
			'xcode_settings': {
				'SYMROOT': '<(DEPTH)/out/xcodebuild/<(os)',
				'SDKROOT': 'macosx',
			},
		}],
	],
}
