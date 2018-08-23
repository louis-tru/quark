{
	'variables': {
		'media%': 1,
		'output%': '',
		'ar%': 'ar',
		'ranlib%': 'ranlib',
		'library%': 'static_library',
		'ff_install_dir':  '<(output)/obj.target/ffmpeg',
		'ff_product_path%': '<(ff_install_dir)/libffmpeg.a',
	},
	'targets': [{
		'target_name': 'ffmpeg',
		'type': 'none',
		'direct_dependent_settings': {
			'include_dirs': [ '<(ff_install_dir)/include' ],
			'defines': [ '__STDC_CONSTANT_MACROS', ],
		},
		'dependencies': [
			'../../node/deps/openssl/openssl.gyp:openssl',
			'../../node/deps/zlib/zlib.gyp:zlib',
			'ffmpeg_compile',
		],
		'sources': [
			'libavutil/avutil.h',
			'libavformat/avformat.h',
			'libswresample/swresample.h',
			'libavcodec/avcodec.h',
		],
		'link_settings': { 
			'libraries': [ 
				'<(ff_product_path)',
			]
		},
		'conditions': [
			['os in "ios osx"', {
				'link_settings': { 
					'libraries': [
						'$(SDKROOT)/System/Library/Frameworks/AudioToolbox.framework',
						'$(SDKROOT)/System/Library/Frameworks/CoreVideo.framework',
						'$(SDKROOT)/System/Library/Frameworks/VideoToolbox.framework',
						'$(SDKROOT)/System/Library/Frameworks/CoreMedia.framework',
						'$(SDKROOT)/usr/lib/libiconv.tbd',
						'$(SDKROOT)/usr/lib/libbz2.tbd',
						'$(SDKROOT)/usr/lib/libz.tbd',
					],
				},
			}],
			['os=="linux"', {
				'link_settings': {
					'libraries': [
						'-lbz2',
						'-lz',
					],
				},
			}],
		],
	}, 
	{
		'target_name': 'ffmpeg_compile',
		'type': 'none',
		'actions': [{
			'action_name': 'ffmpeg_compile',
			'inputs': [ 'config.h' ],
			'outputs': [ '<(ff_product_path)' ],
			'conditions': [
				['media==1', {
					'action': [ 'sh', '-c', 
						'export PATH=<(tools):<(bin):${PATH};'
						'export INSTALL_DIR=<(ff_install_dir);'
						'export PRODUCT_PATH=<(ff_product_path);'
						'export AR=<(ar);'
						'export RANLIB=<(ranlib);'
						'sh ../../tools/build_ffmpeg.sh'
					],
				}, {
					'action': ['echo', 'skip ffmpeg compile'],
				}]
			],
		}],
	}],
}