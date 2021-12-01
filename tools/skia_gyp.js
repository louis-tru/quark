
var fs = require('fs');
var path = require('path');

function gen_gyp() {

	var reg = /path = "?([^"\s;]+)"?/img;

	var code = fs.readFileSync(`${__dirname}/../deps/skia/out/all.xcodeproj/project.pbxproj`, 'utf-8');

	var sources = [];

	var skip = [
		'skia-c-example.c',
		'SkFontMgr_custom_empty_factory.cpp',
		'SkFontMgr_empty_factory.cpp',
	];

	var m;

	while ((m = reg.exec(code))) {
		var name = m[1];
		var basename = path.basename(name);
		var extname = path.extname(name);
		if (skip.indexOf(basename) == -1) {
			if (['.cc', '.c', '.h', '.hpp', '.hxx', '.cpp', '.cxx', '.mm', '.m'].indexOf(extname) != -1) {
				sources.push(`../deps/skia/${name}`);
			}
		}
	}

	// console.log(sources.length);

	var gypi = {
		'includes': [
			'../tools/default_target.gypi',
		],
		'targets': [{
			'target_name': 'skia_gyp',
			'type': 'static_library',
			'defines': [
				'SK_HAS_ANDROID_CODEC',
				'SK_ENABLE_DUMP_GPU',
				'SK_DISABLE_AAA',
				'SK_PARAGRAPH_LIBTXT_SPACES_RESOLUTION',
				'SK_LEGACY_INNER_JOINS',
				'SK_DISABLE_LEGACY_SHADERCONTEXT',
				'SK_DISABLE_LOWP_RASTER_PIPELINE',
				'SK_FORCE_RASTER_PIPELINE_BLITTER',
				'SK_GL',
				'SK_DISABLE_EFFECT_DESERIALIZATION',
				'SK_ENABLE_SKSL',
				'SK_ASSUME_GL_ES=1',
				'SK_ENABLE_API_AVAILABLE',
				'SK_GAMMA_APPLY_TO_A8',
				'SKIA_IMPLEMENTATION=1',
				'SK_SUPPORT_PDF',
				'SK_CODEC_DECODES_JPEG',
				'SK_ENCODE_JPEG',
				'SK_USE_LIBGIFCODEC',
				'SK_CODEC_DECODES_PNG',
				'SK_ENCODE_PNG',
				'SK_CODEC_DECODES_RAW',
				'SK_CODEC_DECODES_WEBP',
				'SK_ENCODE_WEBP',
				'SK_XML',
				"C_ARITH_CODING_SUPPORTED",
				"D_ARITH_CODING_SUPPORTED",
				"SK_SHAPER_CORETEXT_AVAILABLE",
				'qDNGDebug=0',
			],
			'dependencies': [],
			'include_dirs': [
				"../deps/skia",
				"../deps/skia/third_party/externals/libgifcodec",
				"../deps/skia/third_party/externals/libjpeg-turbo",
				"../deps/skia/third_party/externals/libpng",
				"../deps/skia/third_party/externals/libwebp",
				"../deps/skia/third_party/externals/libwebp/src",
				"../deps/skia/third_party/libpng",
				"../deps/skia/third_party/externals/dng_sdk/source",
				"../deps/skia/third_party/externals/piex",
				"../deps/skia/include/third_party/skcms",
			],
			'direct_dependent_settings': {
				'include_dirs': [
					'<(output)/obj.target/skia',
					'<(source)/deps/skia',
				],
			},
			"cflags_cc!": [ "-std=c++14" ],
			"cflags_cc": [ "-std=c++17" ],
			'xcode_settings': { 'CLANG_CXX_LANGUAGE_STANDARD': 'c++17' },
			'conditions': [
				['OS=="mac"', {
					'defines': ['SK_METAL'],
					'link_settings': {
						'libraries': [
							'$(SDKROOT)/System/Library/Frameworks/CoreText.framework',
							'$(SDKROOT)/System/Library/Frameworks/Metal.framework',
						],
					},
				}],
				['os=="android"', {
					'defines': ['SK_VULKAN'],
				}],
			],
			sources,
		}]
	};

	fs.writeFileSync(`${__dirname}/../out/skia.gyp`, JSON.stringify(gypi, null, 2));

}

exports.gen_gyp = gen_gyp;