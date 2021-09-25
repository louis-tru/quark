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
		'skia_build': '<(tools)/build_skia.sh',
		'skia_source': '<(source)/deps/skia',
		'skia_install_dir':  '<(output)/obj.target/skia',
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
			'deps/tess2/tess2.gyp:tess2', 
			'deps/freetype2/freetype2.gyp:ft2',
			'deps/tinyxml2/tinyxml2.gyp:tinyxml2',
			'skia',
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
			# views
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
			'layout/video.cc',
			'layout/view.h',
			'layout/view.cc',
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
			'codec/codec.h',
			'codec/codec.cc',
			'codec/codec-tga.cc',
			'codec/codec-pvrtc.cc',
			#
			# 'css/_css.h',
			# 'css/css.h',
			# 'css/cls.cc',
			# 'css/prop.cc',
			# 'css/scope.cc',
			# 'css/sheets.cc',
			#
			'font/_font.h',
			'font/font.h',
			'font/pool.h',
			'font/defaults.cc',
			'font/family.cc',
			'font/font.cc',
			'font/glyph.cc',
			'font/levels.cc',
			'font/pool.cc',
			#
			'render/render.h',
			'render/render.cc',
			'render/gl.h',
			'render/gl.cc',
			'render/metal.h',
			'render/metal.mm',
			#
			'math.h',
			'math.cc',
			'bezier.h',
			'bezier.cc',
			'media/media.h',
			'media/media.cc',
			#
			'_app.h',
			'pre-render.h',
			'pre-render.cc',
			# '_property.h',
			# '_property.cc',
			'render-looper.h',
			'render-looper.cc',
			# '_text-rows.cc',
			# '_text-rows.h',
			'app.h',
			'app.cc',
			# 'fill.h',
			# 'fill.cc',
			'display.h',
			# 'display.cc',
			# 'draw.h',
			# 'draw.cc',
			'errno.h',
			'event.h',
			# 'event.cc',
			'keyboard.h',
			'keyboard.cc',
			# 'text-font.h',
			# 'text-font.cc',
			'texture.h',
			# 'texture.cc',
			'value.h',
		],
		'conditions': [
			['os=="android"', {
				'sources': [
					'platforms/linux-gl.h',
					'platforms/linux-gl.cc',
					'platforms/android-app.cc',
					'platforms/android-keyboard.cc',
					# '../android/org/flare/FlareActivity.java',
					# '../android/org/flare/Android.java',
					# '../android/org/flare/IMEHelper.java',
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
					'codec/codec-gif.cc',
					'codec/codec-jpeg.cc',
					'codec/codec-png.cc',
					'codec/codec-webp.cc',
				],
				'link_settings': {
					'libraries': [
						# '-lz',
					]
				},
			}],
			['OS=="mac"', {
				'sources':[
					'platforms/mac/mac-app.h',
					'platforms/mac/mac-image-codec.mm',
					'platforms/mac/mac-keyboard.mm',
					'platforms/mac/mac-metal.mm',
					'platforms/mac/mac-render.mm',
					'platforms/mac/mac-render.h',
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
					'platforms/ios/ios-app.mm',
					'platforms/ios/ios-gl.mm',
					'platforms/ios/ios-ime-helper.h',
					'platforms/ios/ios-ime-helper.mm',
					'platforms/ios/ios-raster.mm',
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
					'platforms/_linux-gl.h',
					'platforms/linux-gl.cc',
					'platforms/linux-app.cc',
					'platforms/linux-keyboard.cc',
					'platforms/_linux-ime-helper.h',
					'platforms/linux-ime-helper.cc',
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
			'media/audio-player.h',
			'media/audio-player.cc',
			'media/media-codec.h',
			'media/media-codec.cc',
			'media/_media-codec.h',
			'media/_media-codec.cc',
			'media/media-codec-software.cc',
			'media/media-init.cc',
			'layout/video.cc',
		],
		'conditions': [
			['os=="android"', {
				'sources': [
					'platforms/android-media-codec.cc',
					'platforms/android-pcm-player.cc',
					'platforms/android-pcm-audio-track.cc',
				],
				'link_settings': {
					'libraries': [ '-lOpenSLES', '-lmediandk' ],
				},
			}],
			['OS=="mac"', {
				'sources':[
					'platforms/mac/mac-media-codec.mm',
					'platforms/mac/mac-pcm-player.mm',
				],
			}],
			['os=="linux"', {
				'sources': [
					'platforms/linux-media-codec.cc',
					'platforms/linux-pcm-player.cc',
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
		'direct_dependent_settings': {
			'include_dirs': [ '<(skia_install_dir)', '<(skia_source)', ],
		},
		'sources': [
		],
		'actions': [{
			'action_name': 'skia_compile',
			'inputs': [
				'../deps/skia/out/<(output_name)/args.gn',
			],
			'outputs': [
				'../out/<(output_name)/obj.target/skia/libskia.a',
				'../out/<(output_name)/obj.target/skia/skia',
			],
			'action': [
				'<(skia_build)',
				'<(skia_source)',
				'<(skia_source)/out/<(output_name)',
				'<(skia_install_dir)',
			],
		}],
		'link_settings': {
			'libraries': [
				'<(skia_install_dir)/libskia.a',
			]
		},
		'conditions': [
			['os in "ios osx"', {
				'link_settings': {
					'libraries': [
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
