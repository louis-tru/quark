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
			'str.h',
			'str.inl',
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
			'io/_uv.h',
			'io/_uv.cc',
			'io/fs-async.cc',
			'io/fs-path.cc',
			'io/fs-reader.cc',
			'io/fs-sync.cc',
			'io/fs-path.h',
			'io/fs-reader.h',
			'io/fs.h',
			'io/fs.cc',
			'io/stream.h',
			'io/http-cookie.cc',
			'io/http-helper.cc',
			'io/http-uri.cc',
			'io/http.cc',
			'io/http-cookie.h',
			'io/http.h',
			'io/net.h',
			'io/net.cc',
			'loop/_working.h',
			'loop/_working.cc',
			'loop/thread.h',
			'loop/loop.h',
			'loop/loop.cc',
			'loop/private.cc',
			'str.cc',
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
