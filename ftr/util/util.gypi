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
			'string.h',
			'string.inl',
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
			'_uv.h',
			'_uv.cc',
			'fs/fs-async.cc',
			'fs/fs-path.cc',
			'fs/fs-reader.cc',
			'fs/fs-sync.cc',
			'fs/fs-path.h',
			'fs/fs-reader.h',
			'fs/fs.h',
			'fs/fs.cc',
			'stream.h',
			'net/http-cookie.cc',
			'net/http-helper.cc',
			'net/http-uri.cc',
			'net/http.cc',
			'net/http-cookie.h',
			'net/http.h',
			'net/net.h',
			'net/net.cc',
			'loop/_working.h',
			'loop/_working.cc',
			'loop/thread.h',
			'loop/loop.h',
			'loop/loop.cc',
			'loop/private.cc',
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
					'util/_android-jni.h',
					'util/_android-log.h',
					'android-jni.cc',
					'android-log.cc',
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
