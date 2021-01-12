{
	'targets': [
	{
		# 'product_prefix': '',
		# 'product_ext': 'so',
		'target_name': 'ftr-utils',
		'type': 'static_library', # <(output_type)
		'dependencies': [
			'src/ftr/utils/minizip.gyp:minizip',
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
			'../../include/ftr/utils/buffer.h',
			'../../include/ftr/utils/buffer.inl',
			'../../include/ftr/utils/cb.h',
			'../../include/ftr/utils/codec.h',
			'../../include/ftr/utils/container.h',
			'../../include/ftr/utils/error.h',
			'../../include/ftr/utils/event.h',
			'../../include/ftr/utils/fs.h',
			'../../include/ftr/utils/handle.h',
			'../../include/ftr/utils/hash.h',
			'../../include/ftr/utils/http-cookie.h',
			'../../include/ftr/utils/http.h',
			'../../include/ftr/utils/json.h',
			'../../include/ftr/utils/localstorage.h',
			'../../include/ftr/utils/log.h',
			'../../include/ftr/utils/loop.h',
			'../../include/ftr/utils/macros.h',
			'../../include/ftr/utils/net.h',
			'../../include/ftr/utils/number.h',
			'../../include/ftr/utils/object.h',
			'../../include/ftr/utils/os.h',
			'../../include/ftr/utils/util.h',
			'../../include/ftr/utils/zlib.h',
			'../../include/ftr/utils/str.h',
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
		],
		'conditions': [
			['os=="android"', {
				'conditions': [['<(android_api_level)<24', {
					'defines!': [ '_FILE_OFFSET_BITS=64' ],
				}]],
				'sources':[
					'../../android/android.h',
					'../../android/android.cc',
					'../../include/ftr/utils/android-jni.h',
					'../../include/ftr/utils/android-log.h',
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
