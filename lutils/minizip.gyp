{
	'targets': [
	{
		'target_name': 'minizip',
		'type': '<(library)',
		'cflags': [ '-ansi' ],
		'dependencies': [
			'../depe/node/deps/zlib/zlib.gyp:zlib'
		],
		'direct_dependent_settings': {
			'include_dirs': [ 
				'../depe/node/deps/zlib/contrib/minizip', 
				'../depe/node/deps/zlib',
			],
		},
		'include_dirs': [ '../depe/node/deps/zlib', ],
		'sources': [
			'../depe/node/deps/zlib/contrib/minizip/ioapi.c',
			'../depe/node/deps/zlib/contrib/minizip/zip.c',
			'../depe/node/deps/zlib/contrib/minizip/unzip.c',
		],
		'conditions': [
			['os in "osx ios" and use_system_zlib==1', {
				'link_settings': { 
					'libraries': [ '$(SDKROOT)/usr/lib/libz.tbd' ],
					'libraries!': [ '-lz' ],
				},
			},'use_system_zlib==0', {
				'direct_dependent_settings': {
					# 'include_dirs': [ '../depe/node/deps/zlib' ],
				},
			}],
			[ 'os=="win"', {
				'sources': [ '../depe/node/deps/zlib/contrib/minizip/iowin32.c' ]
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
