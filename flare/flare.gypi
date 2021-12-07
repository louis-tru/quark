{
	'variables': {
		'gui_glsl_files': [
			'render/glsl/_version.glsl',
			'render/glsl/_util.glsl',
			'render/glsl/_box.glsl',
			'render/glsl/_box-radius.glsl',
			'render/glsl/sprite.glsl',
			'render/glsl/box-image.glsl',
			'render/glsl/box-yuv420p-image.glsl',
			'render/glsl/box-yuv420sp-image.glsl',
			'render/glsl/box-border.glsl',
			'render/glsl/box-border-radius.glsl',
			'render/glsl/box-background-image.glsl',
			'render/glsl/box-color.glsl',
			'render/glsl/box-shadow.glsl',
			'render/glsl/text-box-color.glsl',
			'render/glsl/text-texture.glsl',
			'render/glsl/text-vertex.glsl',
			'render/glsl/gen-texture.glsl',
		],
		'gui_default_font_files': [
			'font/langou.ttf',
			'font/iconfont.ttf',
		],
	},
	'targets':[
	{
		'target_name': 'flare',
		'type': 'static_library', #<(output_type)
		'include_dirs': [
			'..',
			'../out',
			'../deps/freetype2/include',
			'../deps/tess2/Include',
			'../deps/tinyxml2',
		],
		'dependencies': [
			'flare-util',
			'skia',
			'deps/tess2/tess2.gyp:tess2', 
			'deps/freetype2/freetype2.gyp:ft2',
			'deps/tinyxml2/tinyxml2.gyp:tinyxml2',
		],
		'direct_dependent_settings': {
			'include_dirs': [ '..' ],
			'xcode_settings': {
			 # 'OTHER_LDFLAGS': '-all_load',
			},
			'conditions': [
				['cplusplus_exceptions==1', {
					'xcode_settings': {
						'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
					},
					'cflags_cc': [ '-fexceptions' ],
				}],
				['cplusplus_rtti==1', {
					'xcode_settings': { 
						'GCC_ENABLE_CPP_RTTI': 'YES',
					},
					'cflags_cc': [ '-frtti', ],
				}],
			],
		}, # direct_dependent_settings
		'sources': [
			'layout/box.h',
			'layout/box.cc',
			'layout/flex.h',
			'layout/flex.cc',
			'layout/flow.h',
			'layout/flow.cc',
			'layout/image.h',
			'layout/image.cc',
			'layout/input.h',
			'layout/input.cc',
			'layout/label.h',
			'layout/label.cc',
			'layout/layout.h',
			'layout/layout.cc',
			'layout/root.h',
			'layout/root.cc',
			'layout/scroll.h',
			'layout/scroll.cc',
			'layout/text.h',
			'layout/text.cc',
			'layout/video.h',
			'layout/view.h',
			'layout/view.cc',
			'codec/codec.h',
			'codec/codec.cc',
			'codec/codec_tga.cc',
			'codec/codec_pvrtc.cc',
			'font/_font.h',
			'font/font.h',
			'font/pool.h',
			'font/defaults.cc',
			'font/family.cc',
			'font/font.cc',
			'font/glyph.cc',
			'font/levels.cc',
			'font/pool.cc',
			'render/render.h',
			'render/render.cc',
			'render/gl.h',
			'render/gl.cc',
			'render/metal.h',
			'render/metal.mm',
			'math.h',
			'math.cc',
			'bezier.h',
			'bezier.cc',
			'media/media.h',
			'media/media.cc',
			'app.inl',
			'pre_render.h',
			'pre_render.cc',
			'app.h',
			'app.cc',
			'display.h',
			'display.cc',
			'errno.h',
			'event.h',
			'event.cc',
			'keyboard.h',
			'keyboard.cc',
			'image_source.h',
			'image_source.cc',
			'value.h',
			'os/os.h',
			'os/os.cc',
			#
			# 'property.h',
			# 'property.cc',
			# 'text-rows.cc',
			# 'text-rows.h',
			'fill.h',
			'fill.cc',
			# 'text-font.h',
			# 'text-font.cc',
			#
			# 'action/_action.h',
			# 'action/action.h',
			# 'action/group.h',
			# 'action/keyframe.h',
			# 'action/action.cc',
			# 'action/center.cc',
			# 'action/group.cc',
			# 'action/keyframe.cc',
			# 'action/_property.h',
			# 'action/property.cc',
			#
			# 'css/_css.h',
			# 'css/css.h',
			# 'css/cls.cc',
			# 'css/prop.cc',
			# 'css/scope.cc',
			# 'css/sheets.cc',
			#
		],
		'conditions': [
			['OS=="mac" and project=="xcode"', {
				'dependencies': [ 'out/skia.gyp:skia_gyp' ], # debug in xcode
			}, {
				'dependencies': [ 'skia' ],
			}],
			['os=="android"', {
				'sources': [
					'platforms/unix/unix_gl.h',
					'platforms/unix/unix_gl.cc',
					'platforms/android/android_app.cc',
					'platforms/android/android_keyboard.cc',
					# 'os/android/org/flare/Activity.java',
					# 'os/android/org/flare/API.java',
					# 'os/android/org/flare/IMEHelper.java',
					'os/android/android_api.h',
					'os/android/android_api.cc',
					'os/android/android_os.cc',
				],
				'link_settings': {
					'libraries': [
						'-lGLESv3',
						'-lEGL',
						'-lz', '-landroid', '-llog',
					],
				},
			}],
			['OS!="mac"', {
				'dependencies': [
					'deps/libgif/libgif.gyp:libgif', 
					'deps/libjpeg/libjpeg.gyp:libjpeg', 
					'deps/libpng/libpng.gyp:libpng',
					'deps/libwebp/libwebp.gyp:libwebp',
				],
				'sources': [
					'codec/codec_gif.cc',
					'codec/codec_jpeg.cc',
					'codec/codec_png.cc',
					'codec/codec_webp.cc',
				],
				'link_settings': {
					'libraries': [
						# '-lz',
					]
				},
			}],
			['OS=="mac"', {
				'dependencies': [
					'deps/reachability/reachability.gyp:reachability',
				],
				'sources':[
					'platforms/apple/apple_app.h',
					'platforms/apple/apple_image_codec.mm',
					'platforms/apple/apple_keyboard.mm',
					'platforms/apple/apple_metal.mm',
					'platforms/apple/apple_render.mm',
					'platforms/apple/apple_render.h',
					'os/apple/apple_os.mm',
				],
				'link_settings': {
					'libraries': [
						# '$(SDKROOT)/usr/lib/libz.tbd',
						'$(SDKROOT)/System/Library/Frameworks/CoreGraphics.framework',
						'$(SDKROOT)/System/Library/Frameworks/QuartzCore.framework',
					]
				},
			}],
			['os=="ios"', {
				'sources':[
					'platforms/ios/ios_app.mm',
					'platforms/ios/ios_gl.mm',
					'platforms/ios/ios_ime_helper.h',
					'platforms/ios/ios_ime_helper.mm',
					'platforms/ios/ios_raster.mm',
				],
				'link_settings': {
					'libraries': [
						'$(SDKROOT)/System/Library/Frameworks/OpenGLES.framework',
						'$(SDKROOT)/System/Library/Frameworks/UIKit.framework',
						'$(SDKROOT)/System/Library/Frameworks/MessageUI.framework',
					]
				},
			}],
			['os=="osx"', {
				'sources': [
					'platforms/osx/osx-app.mm',
					'platforms/osx/osx-gl.mm',
				],
				'link_settings': {
					'libraries': [
						'$(SDKROOT)/System/Library/Frameworks/OpenGL.framework',
						'$(SDKROOT)/System/Library/Frameworks/AppKit.framework',
						'$(SDKROOT)/System/Library/Frameworks/IOKit.framework',
					]
				},
			}],
			['os=="linux"', {
				'sources': [
					'platforms/linux/linux_gl.h',
					'platforms/linux/linux_gl.cc',
					'platforms/linux/linux_app.cc',
					'platforms/linux/linux_keyboard.cc',
					'platforms/linux/linux_ime_helper.h',
					'platforms/linux/linux_ime_helper.cc',
					'os/linux/linux_os.cc',
				],
				'link_settings': {
					'libraries': [
						'-lGLESv2', '-lEGL', '-lX11', '-lXi', '-lasound',
					],
				},
			}],
			# conditions end
		],
		'actions': [
			{
				'action_name': 'gen_font_natives',
				'inputs': [
					'../tools/gen-font-natives.js',
					'<@(gui_default_font_files)',
				],
				'outputs': [
					'../out/native-font.h',
					'../out/native-font.cc',
				],
				'action': [
					'<(node)',
					'<@(_inputs)',
					'<@(_outputs)',
				],
				'process_outputs_as_sources': 1,
			},
		],
		# end
	},
	{
		'target_name': 'flare-media',
		'type': 'static_library', #<(output_type)
		'dependencies': [
			'flare',
			'deps/ffmpeg/ffmpeg.gyp:ffmpeg',
		],
		'sources': [
			'media/pcm.h',
			'media/audio_player.h',
			'media/audio_player.cc',
			'media/media_codec.h',
			'media/media_codec.cc',
			'media/media_codec_inl.h',
			'media/media_codec_inl.cc',
			'media/media_codec_software.cc',
			'media/media_init.cc',
			'layout/video.cc',
		],
		'conditions': [
			['os=="android"', {
				'sources': [
					'platforms/android_media_codec.cc',
					'platforms/android_pcm_player.cc',
					'platforms/android_pcm_audio_track.cc',
				],
				'link_settings': {
					'libraries': [ '-lOpenSLES', '-lmediandk' ],
				},
			}],
			['OS=="mac"', {
				'sources':[
					'platforms/apple/apple_media_codec.mm',
					'platforms/apple/apple_pcm_player.mm',
				],
			}],
			['os=="linux"', {
				'sources': [
					'platforms/linux_media_codec.cc',
					'platforms/linux_pcm_player.cc',
				],
				'link_settings': { 
					'libraries': [ '-lasound' ],
				},
			}],
		],
	},
	{
		'target_name': 'skia',
		'type': 'none',
		"direct_dependent_settings": {
			"include_dirs": [
				"<(output)/obj.target/skia",
				"<(source)/deps/skia"
			]
		},
		'sources': [],
		'conditions': [
			['OS=="mac" and project=="xcode"', { # use skia_gyp
				'dependencies': [ 'out/skia.gyp:skia_gyp' ], # debug in xcode
			}, { # use ninja build
				'actions': [{
					'action_name': 'skia_compile',
					'inputs': [
						'../out/<(output_name)/obj.target/skia/args.gn',
					],
					'outputs': [
						'../out/<(output_name)/obj.target/skia/libskia.a',
						'../out/<(output_name)/obj.target/skia/skia',
						'../deps/skia',
					],
					'action': [
						'<(tools)/build_skia.sh',
						'<(output)/obj.target/skia',
					],
				}],
				'link_settings': {
					'libraries': [
						'<(output)/obj.target/skia/libskia.a',
					]
				},
			}],
			# common
			['os in "ios osx"', {
				'link_settings': {
					'libraries': [
						'$(SDKROOT)/System/Library/Frameworks/CoreText.framework',
						'$(SDKROOT)/System/Library/Frameworks/Metal.framework',
					],
				},
			}],
			['os=="android"', {
				'link_settings': {
				},
			}],
			['os=="linux"', {
				'link_settings': {
				},
			}],
		],
	},
	]
}
