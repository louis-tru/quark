{
	'targets': [
	{
		# 'product_prefix': '',
		# 'product_ext': 'so',
		'target_name': 'flare-util',
		'type': 'static_library',
		'dependencies': [
			'minizip',
			'deps/bplus/bplus.gyp:bplus',
			'deps/libuv/libuv.gyp:libuv',
			'deps/node/deps/openssl/openssl.gyp:openssl',
			'deps/node/deps/http_parser/http_parser.gyp:http_parser',
		],
		'direct_dependent_settings': {
			'include_dirs': [ '../..' ],
			'mac_bundle_resources': [],
		},
		'include_dirs': [
			'../..',
			'../../deps/rapidjson/include',
			'../../deps/bplus/include',
			'../../deps/node/deps/zlib',
			'../../deps/node/deps/zlib/contrib/minizip',
		],
		'sources': [
			'../../Makefile',
			'../../README.md',
			'../../configure',
			'../../tools/configure.js',
			# h
			'../version.h',
			'errno.h',
			'uv.h',
			'working.h',
			'fs.h',
			'http.h',
			'net.h',
			'loop.h',
			'thread.h',
			'string.h',
			'array.h',
			'cb.h',
			'codec.h',
			'error.h',
			'event.h',
			'handle.h',
			'hash.h',
			'json.h',
			'storage.h',
			'log.h',
			'macros.h',
			'numbers.h',
			'object.h',
			'os.h',
			'util.h',
			'zlib.h',
			'iterator.h',
			'list.h',
			'dict.h',
			'dict.cc',
			# cc
			'uv.cc',
			'working.cc',
			'array.cc',
			'fs-async.cc',
			'fs-path.cc',
			'fs-reader.cc',
			'fs-sync.cc',
			'fs.cc',
			'stream.h',
			'http-cookie.cc',
			'http-helper.cc',
			'http-uri.cc',
			'http.cc',
			'net.cc',
			'thread.cc',
			'loop.cc',
			'loop-private.cc',
			'string.cc',
			'cb.cc',
			'codec.cc',
			'date.cc',
			'error.cc',
			'json.cc',
			'storage.cc',
			'object.cc',
			'os.cc',
			'util.cc',
			'zlib.cc',
			'numbers.cc',
			'hash.cc',
			'log.cc',
		],
		'conditions': [
			['os=="android"', {
				'conditions': [['<(android_api_level)<24', {
					'defines!': [ '_FILE_OFFSET_BITS=64' ],
				}]],
				'sources':[
					'../../android/android.h',
					'../../android/android.cc',
					'platforms/android-jni.h',
					'platforms/android-log.h',
					'platforms/android-jni.cc',
					'platforms/android-log.cc',
					'platforms/android-path.cc',
					'platforms/android-os.cc',
				],
				'link_settings': {
					'libraries': [
						'-latomic', 
						'-llog', 
						'-landroid',
						'-lz',
					],
				},
			}],
			['os=="linux"', {
				'sources': [
					'platforms/unix-path.cc',
					'platforms/unix-os.cc',
				],
				'link_settings': {
					'libraries': [
						'-lz',
					]
				},
			}],
			['OS=="mac"', {
				'dependencies': [
					'deps/reachability/reachability.gyp:reachability',
				],
				'sources': [
					'platforms/mac-path.mm',
					'platforms/mac-os.mm',
				],
				'link_settings': {
					'libraries': [
						'$(SDKROOT)/usr/lib/libz.tbd',
					]
				},
			}],
		],
	}, 
	{
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
	}
	],
}
