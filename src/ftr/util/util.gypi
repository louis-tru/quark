{
	'targets': [
	{
		# 'product_prefix': '',
		# 'product_ext': 'so',
		'target_name': 'ftr-util',
		'type': 'static_library', # <(output_type)
		'dependencies': [
			'src/ftr/util/minizip.gyp:minizip',
			'depe/bplus/bplus.gyp:bplus',
			'depe/libuv/libuv.gyp:libuv',
			'depe/node/deps/openssl/openssl.gyp:openssl',
			'depe/node/deps/http_parser/http_parser.gyp:http_parser',
		],
		'direct_dependent_settings': {
			'include_dirs': [ '../../include' ],
			'mac_bundle_resources': [
			],
		},
		'include_dirs': [
			'../../include',
			'../../../depe/rapidjson/include',
			'../../../depe/bplus/include',
			'../../../depe/node/deps/zlib',
			'../../../depe/node/deps/zlib/contrib/minizip',
		],
		'sources': [
			'../../../Makefile',
			'../../../README.md',
			'../../../configure',
			'../../../tools/configure.js',
			# h
			'../../include/ftr/version.h',
			'../../include/ftr/errno.h',
			'../../include/ftr/util/buffer.h',
			'../../include/ftr/util/buffer.inl',
			'../../include/ftr/util/cb.h',
			'../../include/ftr/util/codec.h',
			'../../include/ftr/util/error.h',
			'../../include/ftr/util/event.h',
			'../../include/ftr/util/fs.h',
			'../../include/ftr/util/handle.h',
			'../../include/ftr/util/hash.h',
			'../../include/ftr/util/http-cookie.h',
			'../../include/ftr/util/http.h',
			'../../include/ftr/util/json.h',
			'../../include/ftr/util/localstorage.h',
			'../../include/ftr/util/log.h',
			'../../include/ftr/util/loop.h',
			'../../include/ftr/util/macros.h',
			'../../include/ftr/util/net.h',
			'../../include/ftr/util/numbers.h',
			'../../include/ftr/util/object.h',
			'../../include/ftr/util/os.h',
			'../../include/ftr/util/util.h',
			'../../include/ftr/util/zlib.h',
			'../../include/ftr/util/str.h',
			'../../include/ftr/util/str.inl',
			# cc
			'buffer.cc',
			'cb.cc',
			'codec.cc',
			'date.cc',
			'error.cc',
			'fs-async.cc',
			'fs-file.cc',
			'fs-reader.cc',
			'fs-sync.cc',
			'fs.cc',
			'http-cookie.cc',
			'http-helper.cc',
			'http-uri.cc',
			'http.cc',
			'json.cc',
			'localstorage.cc',
			'loop-private.cc',
			'loop.cc',
			'loop.h',
			'net.cc',
			'object.cc',
			'os.cc',
			'util.cc',
			'uv.h',
			'zlib.cc',
			'str.cc',
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
					'../../include/ftr/util/android-jni.h',
					'../../include/ftr/util/android-log.h',
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
