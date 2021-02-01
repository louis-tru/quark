{
	'targets': [
	{
		# 'product_prefix': '',
		# 'product_ext': 'so',
		'target_name': 'ftr-util',
		'type': 'static_library', # <(output_type)
		'dependencies': [
			'ftr/util/minizip.gyp:minizip',
			'depe/bplus/bplus.gyp:bplus',
			'depe/libuv/libuv.gyp:libuv',
			'depe/node/deps/openssl/openssl.gyp:openssl',
			'depe/node/deps/http_parser/http_parser.gyp:http_parser',
		],
		'direct_dependent_settings': {
			'include_dirs': [ '../..' ],
			'mac_bundle_resources': [
			],
		},
		'include_dirs': [
			'../..',
			'../../depe/rapidjson/include',
			'../../depe/bplus/include',
			'../../depe/node/deps/zlib',
			'../../depe/node/deps/zlib/contrib/minizip',
		],
		'sources': [
			'../../Makefile',
			'../../README.md',
			'../../configure',
			'../../tools/configure.js',
			# h
			'../version.h',
			'errno.h',
			'_uv.h',
			'_working.h',
			'fs.h',
			'http.h',
			'net.h',
			'loop.h',
			'thread.h',
			'string.h',
			'buffer.h',
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
			# cc
			'_uv.cc',
			'_working.cc',
			'buffer.cc',
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
					'_android-jni.h',
					'_android-log.h',
					'_android-jni.cc',
					'_android-log.cc',
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
					'platforms/linux-path.cc',
					'platforms/linux-os.cc',
				],
				'link_settings': {
					'libraries': [
						'-lz',
					]
				},
			}],
			['OS=="mac"', {
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
	],
}
