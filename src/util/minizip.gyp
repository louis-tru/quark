{
	'targets': [{
		'target_name': 'minizip',
		'type': 'static_library',
		'cflags': [ '-ansi' ],
		'direct_dependent_settings': {
			'include_dirs': [ 
				'../../deps/node/deps/zlib/contrib/minizip', 
				'../../deps/node/deps/zlib',
			],
		},
		'include_dirs': [ '../deps/node/deps/zlib', ],
		'sources': [
			'../../deps/node/deps/zlib/contrib/minizip/ioapi.c',
			'../../deps/node/deps/zlib/contrib/minizip/zip.c',
			'../../deps/node/deps/zlib/contrib/minizip/unzip.c',
		],
		'conditions': [
			['use_system_zlib==1', {
				'defines': [ 'USE_SYSTEM_ZLIB' ],
				'link_settings': {
					'libraries': [ '-lz' ],
				},
				'direct_dependent_settings': {
					'defines': [ 'USE_SYSTEM_ZLIB' ],
				},
			}, {
				'dependencies': [
					'deps/node/deps/zlib/zlib.gyp:zlib'
				],
			}],
			['os in "osx ios" and use_system_zlib==1', {
				'link_settings': {
					'libraries': [ '$(SDKROOT)/usr/lib/libz.tbd' ],
					'libraries!': [ '-lz' ],
				},
			}],
			[ 'os=="win"', {
				'sources': [ '../../deps/node/deps/zlib/contrib/minizip/iowin32.c' ]
			},{
				'cflags!': [ '-ansi' ],
			}],
			[ 'os in "osx ios"', {
				'xcode_settings': {
					'GCC_C_LANGUAGE_STANDARD': 'ansi',
				},
			}],
			[ 'os in "android osx ios"', {
				'defines': [ 'USE_FILE32API', ],
			}],
		]
	}],
}
