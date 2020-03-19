{
	'variables': {
		'OS%': 'mac',
		'os%': 'ios',
		'library%': 'static_library',
		'clang%': 0,
		'without_visibility_hidden%': 0,
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
					# 'GCC_PREPROCESSOR_DEFINITIONS[sdk=iphoneos*]': [ 'USE_JSC=0' ],
				},
			},
			'Release': {
				'defines': [ 'NDEBUG' ],
				'cflags': [
					'-O3', 
					'-ffunction-sections', 
					'-fdata-sections', 
					# '-fvisibility=hidden',
					'-fomit-frame-pointer',
				],
				'cflags_cc': [
					'-fvisibility-inlines-hidden',
				],
				'ldflags': [ '-s' ],
				'xcode_settings': {
					'GCC_OPTIMIZATION_LEVEL': '3',  # -O3
					'GCC_STRICT_ALIASING': 'YES',
					'ONLY_ACTIVE_ARCH': 'NO',
				},
				'conditions': [
					['os=="android" and clang==0', {
						'cflags!': [ '-O3' ],
						'cflags': [ '-O2' ],
					}],
					['without_visibility_hidden==1', {
						'cflags!': [ '-fvisibility=hidden' ],
					}],
				],
			},
		},
		'type': '<(library)',
		'cflags!': ['-Werror'],
		'xcode_settings': {
			'CLANG_CXX_LANGUAGE_STANDARD': 'c++0x',   # -std=c++0x
			'CLANG_CXX_LIBRARY': 'libc++',            # c++11 libc support
			'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',       # -fexceptions
			'GCC_ENABLE_CPP_RTTI': 'YES',             # -frtti / -fno-rtti
		},
		'cflags_cc': [ '-std=c++0x', '-fexceptions', '-frtti', ],
		'defines': [
			'NX_USING_SHARED=1',
			'USING_UV_SHARED=1',
			'USING_V8_SHARED=1',
			'USING_V8_PLATFORM_SHARED=1',
		],
		'conditions': [
			['os=="android"', {
				'ldflags': [
					'-shared',
					'-Wl,--gc-sections',  # Discard Unused Functions with gc-sections
				],
				'conditions': [
					['clang==0', {
						'cflags': [ '-funswitch-loops', '-finline-limit=64' ],
					},{
						'cflags!': [ '-Wno-old-style-declaration' ],
						'cflags': [ '-fPIC' ],
					}],
				],
				'link_settings': {
					'libraries': [
						'-llog', 
						'-landroid',
						'-lnxkit',
						'-lngui',
						'-lngui-media',
						'-lngui-v8',
						'-lngui-js',
						'-lngui-node',
					],
					'library_dirs': [
						'<(DEPTH)/out/libs/android/jniLibs/${ANDROID_ABI}',
					],
				},
				'include_dirs': [ '<(DEPTH)/out/libs/include' ],
			}],
			['os=="linux"', {
				'cflags': [
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
				'ldflags': [
					'-pthread', #'-rdynamic',
				],
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
					'VALID_ARCHS': ['arm64'],
				},
				'cflags_cc': [ '-stdlib=libc++' ],
				'link_settings': {
					'libraries': [
						'$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
						'$(SDKROOT)/System/Library/Frameworks/UIKit.framework',
						'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)/nxkit.framework',
						'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)/ngui.framework',
						'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)/ngui-media.framework',
						'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)/ngui-v8.framework',
						'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)/ngui-js.framework',
						'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)/ngui-node.framework',
					],
				},
				'mac_framework_dirs': [
					'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)',
				],
				'mac_bundle_frameworks': [
					'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)/nxkit.framework',
					'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)/ngui.framework',
					'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)/ngui-media.framework',
					'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)/ngui-v8.framework',
					'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)/ngui-js.framework',
					'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)/ngui-node.framework',
				],
				'include_dirs': [ '<(DEPTH)/out/libs/include' ],
			}],
			['os=="osx"', {
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
					'AECHS': ['$(ARCHS_STANDARD)'],           # 'ARCHS': 'x86_64',
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
					'libraries': [ '$(SDKROOT)/System/Library/Frameworks/Foundation.framework' ],
				},
			}],
		],
	},

	'_targets': [{
		'target_name': 'libngui',
		'type': 'none',
		'direct_dependent_settings': {
			'defines': [ 
				'NX_USING_SHARED=1',
				'USING_UV_SHARED=1',
				'USING_V8_SHARED=1',
				'USING_V8_PLATFORM_SHARED=1',
			],
			'xcode_settings': {
				'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
				'GCC_ENABLE_CPP_RTTI': 'YES', 
			},
			'cflags_cc': [ '-fexceptions', '-frtti', ], 
		},
		'conditions': [
			['os=="android"', {
				'link_settings': {
					'libraries': [
						'-llog', 
						'-landroid',
						'-lnxkit',
						'-lngui',
						'-lngui-media',
						'-lngui-v8',
						'-lngui-js',
						'-lngui-node',
					],
					'library_dirs': [
						'<(DEPTH)/out/libs/android/jniLibs/${ANDROID_ABI}',
					],
				},
				'direct_dependent_settings': {
					'include_dirs': [ '<(DEPTH)/out/libs/include' ],
				},
			}],
			['os=="ios"', {
				'link_settings': {
					'libraries': [
						'$(SDKROOT)/System/Library/Frameworks/UIKit.framework',
						'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)/nxkit.framework',
						'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)/ngui.framework',
						'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)/ngui-media.framework',
						'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)/ngui-v8.framework',
						'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)/ngui-js.framework',
						'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)/ngui-node.framework',
					],
				},
				'direct_dependent_settings': {
					'mac_framework_dirs': [
						'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)',
					],
					'mac_bundle_frameworks': [
						'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)/nxkit.framework',
						'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)/ngui.framework',
						'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)/ngui-media.framework',
						'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)/ngui-v8.framework',
						'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)/ngui-js.framework',
						'<(DEPTH)/out/libs/ios/Frameworks/$(PLATFORM_NAME)/$(CONFIGURATION)/ngui-node.framework',
					],
					'include_dirs': [ '<(DEPTH)/out/libs/include' ],
				},
			}],
		],
	}],

	'conditions': [
		['os=="ios"', {
			'xcode_settings': {
				'SYMROOT': '<(DEPTH)/out/xcodebuild/<(os)',
				'SDKROOT': 'iphoneos',
			},
		}, 
		'os=="osx"', {
			'xcode_settings': {
				'SYMROOT': '<(DEPTH)/out/xcodebuild/<(os)',
				'SDKROOT': 'macosx',
			},
		}],
	],
}
