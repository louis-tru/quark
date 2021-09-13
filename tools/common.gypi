{
	'variables': {
		'configuration%': 'Release',
		'arch%': 'x86',
		'arch_name%': '<(arch)',
		'suffix%': '<(arch)',
		'byteorder%': '<(node_byteorder)',
		'clang%': 0,
		'is_clang': '<(clang)',
		'python%': 'python',
		'host_node%': 'node',
		'node%': '<(host_node)',
		'debug%': 0,
		'project%': 'make',
		'media%': 1,
		'std_cpp%': 'c++14',
		'gcc_version%': 0,
		'llvm_version%': 0,
		'xcode_version%': 0,
		'MSVS_VERSION%': 0,
		'gas_version%': 0,
		'sysroot': '<(build_sysroot)',
		'SDKROOT': '<(build_sysroot)',
		'build_tools': '<(source)/tools',
		'bin': '<(build_bin)', 
		'tools': '<(build_tools)',
		'cc%': 'gcc', 
		'cxx%': 'g++', 
		'ar%': 'ar', 
		'ranlib%': 'ranlib', 
		'strip%': 'strip',
		'android_api_level%': 21,
		'output_name%': '',
		'output%': '<(PRODUCT_DIR)',
		'version_min%': '',
		'android_abi%': '',
		'use_system_zlib%': 0,
		'use_v8%': 1,
		'without_visibility_hidden%': 0,
		'without_embed_bitcode%': 0,
		'cross_compiling%': 0,

		############################# dependents set ##################
		
		'target_arch%': 'ia32',           # set v8's target architecture
		'host_arch%': 'ia32',             # set v8's host architecture
		'host_os%': 'mac',
		'library%': 'static_library',     # allow override to 'shared_library' for DLL/.so builds
		'component': '<(library)',        # NB. these names match with what V8 expects
		'uv_library': '<(library)',       #
		'asan%': 0,
		'werror': '',                     # Turn off -Werror in V8 build.
		'msvs_multi_core_compile': '0',   # we do enable multicore compiles, but not using the V8 way
		'want_separate_host_toolset%': 0, # V8 should not build target and host
		'v8_enable_gdbjit%': 0,
		'v8_enable_i18n_support%': 0,
		'v8_no_strict_aliasing%': 1,
		'v8_use_snapshot%': 'false',      # Enable the snapshot feature, for fast context creation.
		'v8_optimized_debug%': 0,         # Default to -O0 for debug builds.
		'v8_promise_internal_field_count%': 1,
		'v8_enable_disassembler': 0,      # Enable disassembler
		'v8_use_external_startup_data': 0,# Don't bake anything extra into the snapshot.
		'v8_postmortem_support%': 'false',
		'v8_enable_inspector%': 0,
		'icu_use_data_file_flag': 0,      # Don't use ICU data file (icudtl.dat) from V8, we use our own.
		'icu_small%': 'true',
		
		# Default ARM variable settings.
		'arm_version': 'default',
		'arm_float_abi': 'default',
		'arm_thumb': 'default',
		'armv7%': 0,
		'armv7s%': 0,  # armv7s form apple iphone
		'arm_neon%': 0,
		'arm_vfp%': 'vfpv3',
		'arm_fpu%': 'neon',
		'arm64%': 0,
		
		# Default MIPS variable settings.
		'mips_arch_variant%': 'r2',
		# Possible values fp32, fp64, fpxx.
		# fp32 - 32 32-bit FPU registers are available, doubles are placed in
		#        register pairs.
		# fp64 - 32 64-bit FPU registers are available.
		# fpxx - compatibility mode, it chooses fp32 or fp64 depending on runtime
		#        detection
		'mips_fpu_mode%': 'fp32',

		# node js
		'node_use_loader': 0,
		'node_shared%': 'false',
		'node_use_v8_platform%': 'false',
		'node_use_bundled_v8%': 'false',
		'node_module_version%': '',
		'node_target_type': '<(library)',
		'node_tag%': '',
		'node_byteorder%': 'little',
		'node_enable_d8%': 'false',
		'node_enable%': 0,
		'node_release_urlbase%': '',
		'openssl_no_asm%': 0,
		'openssl_fips%': '',
		'debug_http2%': 'false',
		'debug_nghttp2%': 'false',
		'OBJ_DIR%': '<(PRODUCT_DIR)/obj.target',
		'V8_BASE%': '<(PRODUCT_DIR)/obj.target/deps/v8/src/libv8_base.a',

		# conditions
		'conditions': [
			# node js
			['os == "win"', {
				'os_posix': 0,
				'OBJ_DIR': '<(PRODUCT_DIR)/obj',
				'V8_BASE': '<(PRODUCT_DIR)/lib/v8_libbase.lib',
			}, {
				'os_posix': 1,
			}],
			['os not in "win android ios"', {
				'v8_postmortem_support%': 'true',
			}],
			['os in "ios osx"', {
				'OBJ_DIR%': '<(PRODUCT_DIR)/obj.target',
				'V8_BASE': '<(PRODUCT_DIR)/libv8_base.a',
			}],
			['openssl_fips!=""', {
				'OPENSSL_PRODUCT': 'libcrypto.a',
			}, {
				'OPENSSL_PRODUCT': 'libopenssl.a',
			}],
			['library_output=="shared_library"', {
				'node_shared': 'true',
			}],
		],

		##########################################################
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
					'-O3', 
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
					'GCC_OPTIMIZATION_LEVEL': '3',  # -O3
					'GCC_STRICT_ALIASING': 'YES',
					'ONLY_ACTIVE_ARCH': 'YES',
					# 'GCC_SYMBOLS_PRIVATE_EXTERN': 'YES',  # -fvisibility=hidden
				},
				'conditions': [
					['os=="android"', { #  and clang==0
						'cflags!': [ '-O3' ], # Android uses - O3 to make the package larger 
						'cflags': [ '-O2' ],
					}],
					['without_visibility_hidden==1', {
						'cflags!': [ '-fvisibility=hidden' ],
					}],
				],
			}
		},
		'cflags!': ['-Werror'],
		'cflags_cc': [ '-fno-rtti', '-fno-exceptions', '-std=<(std_cpp)' ],
		'xcode_settings': {
			'CLANG_CXX_LANGUAGE_STANDARD': '<(std_cpp)',# -std=c++0x
			'CLANG_CXX_LIBRARY': 'libc++',             # c++11 libc support
				# 'GCC_C_LANGUAGE_STANDARD': 'c99',
		},
		'include_dirs': [
			'..',
			'../deps/libuv/include',
			'../deps/node/deps/openssl/openssl/include',
			'../deps/node/deps/zlib',
			'../deps/node/deps/http_parser',
			'../deps/v8-link/include',
		],
		'conditions': [
			['os=="android"', {
				'cflags': [
					'-fPIC',
					'-pthread',
				],
				'ldflags': [
					'-Wl,--gc-sections',  # Discard Unused Functions with gc-sections
					'-pthread',
					'-rdynamic',
					# '-ddynamiclib', # clang flag
					# '-stdlib=libstdc++', # use libstdc++, clang default use libc++_shared, clang flag
					'-static-libstdc++', # link static-libstdc++, clang default use libc++_shared
				],
				'conditions': [
					['clang==0', {
						'cflags': [ '-funswitch-loops', '-finline-limit=64' ],
					}, {
						'cflags!': [ '-Wno-old-style-declaration' ],
					}],
					['arch=="arm"', { 'cflags': [ '-march=<(arch_name)' ] }],
					['arch=="arm" and arm_vfp!="none"', {
						'cflags': [ '-mfpu=<(arm_vfp)', '-mfloat-abi=softfp' ],
					}],
					['arch=="arm"', { 'ldflags': [ '-Wl,--icf=safe' ] }], # Remove Duplicated Code
				],
				'defines': [
					'_GLIBCXX_USE_C99', 
					'_GLIBCXX_USE_C99_MATH', 
					'_GLIBCXX_USE_C99_MATH_TR1',
					'_GLIBCXX_HAVE_WCSTOF',
					'__ANDROID_API__=<(android_api_level)',
				],
			}],
			['os=="linux"', {
				'include_dirs': [
					'linux/usr/include',
				],
				'defines': [ '__STDC_LIMIT_MACROS' ],
				'cflags': [
					'-fPIC',
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
					'-march=<(arch_name)',
					# '-Wno-misleading-indentation',
				],
				'ldflags': [
					'-pthread',
					'-rdynamic',
					'-L<(build_tools)/linux/usr/lib/<(arch_name)',
				],
				'conditions': [
					['arch=="x86"', { 'cflags': [ '-m32' ], 'cflags!': [ '-march=<(arch_name)' ] }],
					['arch=="x64"', { 'cflags': [ '-m64' ] }],
					['gcc_version>="7.0"', { 'cflags': [ '-Wno-implicit-fallthrough' ] }],
					['gcc_version>="8.0"', { 
						'cflags': [ '-Wno-cast-function-type' ],
						'cflags_cc': [ '-Wno-class-memaccess' ],
					}],
				],
			}],
			['os=="ios"', {
				'cflags': [
					'-miphoneos-version-min=<(version_min)', 
					'-arch <(arch_name)'
					'-isysroot <(sysroot)', 
					# '-fembed-bitcode', #'-fembed-bitcode-marker',
				],
				'cflags_cc': [ '-stdlib=libc++' ],
				'ldflags': [ 
					'-miphoneos-version-min=<(version_min)',
					'-arch <(arch_name)',
				],
				'link_settings': { 
					'libraries!': [ '-lm' ],
					'libraries': [ '$(SDKROOT)/System/Library/Frameworks/Foundation.framework' ],
				},
				'conditions': [
					['arch in "arm arm64"', { 'defines': [ 'USE_SIMULATOR' ]} ], # v8 setting
					['without_embed_bitcode==1', { 
						'cflags': [ 
							#'-fembed-bitcode-marker' 
						]
					}, {
						'cflags': [ '-fembed-bitcode' ]
					}],
				],
				'xcode_settings': {
					'SYMROOT': '<(DEPTH)/out/xcodebuild/<(os).<(suffix)',
					'ALWAYS_SEARCH_USER_PATHS': 'NO',
					'GCC_TREAT_WARNINGS_AS_ERRORS': 'NO',
					'SDKROOT': 'iphoneos',
					'TARGETED_DEVICE_FAMILY': '1,2',
					'IPHONEOS_DEPLOYMENT_TARGET': '<(version_min)',
					'USE_HEADERMAP': 'NO',
					'ARCHS': [ '$(ARCHS_STANDARD)' ],   # 'ARCHS': [ '$(ARCHS_STANDARD_32_BIT)' ],
					'SKIP_INSTALL': 'YES',
					'DEBUG_INFORMATION_FORMAT': 'dwarf', # dwarf-with-dsym
					'CLANG_ENABLE_OBJC_ARC': 'YES',
					'VALID_ARCHS': '<(arch_name)',
					'conditions': [
						['arch in "x86 x64"', { 'SDKROOT': 'iphonesimulator' }],
						['project=="xcode"', {
							# v8 setting
							'GCC_PREPROCESSOR_DEFINITIONS[arch=armv7]': [ '$(inherited)', 'USE_SIMULATOR', ],
							'GCC_PREPROCESSOR_DEFINITIONS[arch=armv7s]': [ '$(inherited)', 'USE_SIMULATOR', ],
							'GCC_PREPROCESSOR_DEFINITIONS[arch=arm64]': [ '$(inherited)', 'USE_SIMULATOR', ],
						}],
						['without_embed_bitcode==0', {
							'ENABLE_BITCODE': 'YES'
						}, {
							'ENABLE_BITCODE': 'NO'
						}]
					],
				},
			}],
			['os=="osx"', {
				'cflags': [ 
					'-mmacosx-version-min=<(version_min)',
					'-arch <(arch_name)',
					'-isysroot <(sysroot)', 
				],
				'cflags_cc': [ '-stdlib=libc++' ],
				'ldflags': [ 
					'-mmacosx-version-min=<(version_min)',
					'-arch <(arch_name)',
				],
				'ldflags!': [ '-pthread', '-s' ],
				'link_settings': { 
					'libraries!': [ '-lm' ],
					'libraries': [ 
						'$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
						'$(SDKROOT)/System/Library/Frameworks/CoreFoundation.framework',
					],
				},
				'xcode_settings': {
					'SYMROOT': '<(DEPTH)/out/xcodebuild/<(os).<(suffix)',
					'ALWAYS_SEARCH_USER_PATHS': 'NO',
					'GCC_TREAT_WARNINGS_AS_ERRORS': 'NO',
					'SDKROOT': 'macosx',
					'GCC_CW_ASM_SYNTAX': 'NO',                # No -fasm-blocks
					'GCC_DYNAMIC_NO_PIC': 'NO',               # No -mdynamic-no-pic
																										# (Equivalent to -fPIC)
					'GCC_ENABLE_PASCAL_STRINGS': 'NO',        # No -mpascal-strings
					'GCC_THREADSAFE_STATICS': 'NO',           # -fno-threadsafe-statics
					'PREBINDING': 'NO',                       # No -Wl,-prebind
					'MACOSX_DEPLOYMENT_TARGET': '<(version_min)',  # -mmacosx-version-min=10.7
					'USE_HEADERMAP': 'NO',
					'AECHS': ['$(ARCHS_STANDARD)'],           # 'ARCHS': 'x86_64',
					'SKIP_INSTALL': 'YES',
					'DEBUG_INFORMATION_FORMAT': 'dwarf',      # dwarf-with-dsym
					'ENABLE_BITCODE': 'NO',
					'CLANG_ENABLE_OBJC_ARC': 'YES',
					'VALID_ARCHS': '<(arch_name)',
				},
			}],
			['OS=="mac"', {
				'libraries!': ['-framework CoreFoundation', '-lz'],
				'xcode_settings': {
					'GCC_ENABLE_CPP_EXCEPTIONS': 'NO',   # -fno-exceptions
					'GCC_ENABLE_CPP_RTTI':       'NO',   # -fno-rtti
				},
			}],
			['use_v8==0 and os=="ios"', {
				'defines': [ 'USE_JSC=1' ],
			},{
				'defines': [ 'USE_JSC=0' ],
			}],
		],
		'target_conditions': [
			# shared all public symbol
			['_target_name in "node"', {
				'defines': [ 'NODE_SHARED_MODE=1' ],
				# 'dependencies!': [ 'v8_inspector_compress_protocol_json#host' ],
			}],
			['_target_name in "openssl http_parser zlib"', {
				'cflags!': ['-fvisibility=hidden'],
			}],
			['_target_name in "libuv"', {
				'defines': [ 'BUILDING_UV_SHARED=1' ],
			}],
			['_target_name in "v8_libplatform v8_libbase v8_base"', {
				'defines': [ 
					'BUILDING_V8_SHARED=1', 
					'BUILDING_V8_PLATFORM_SHARED=1',
				],
			}],
			['_target_name in "v8_external_snapshot"', {
				'sources': [  'useless.c' ],
			}],
			['_target_name in "mksnapshot"', {
				'conditions': [
					['os=="android"', {
						'ldflags': [ '-llog' ],
					}]
				]
			}],
		],
	},

	'conditions': [
		['os=="ios"', {
			'xcode_settings': {
				'SYMROOT': '<(DEPTH)/out/xcodebuild/<(os).<(suffix)',
				'SDKROOT': 'iphoneos',
			},
		}, 
		'os=="osx"', {
			'xcode_settings': {
				'SYMROOT': '<(DEPTH)/out/xcodebuild/<(os).<(suffix)',
				'SDKROOT': 'macosx',
			},
		}],
	],
}
