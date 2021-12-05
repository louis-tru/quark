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
			'codec/codec-tga.cc',
			'codec/codec-pvrtc.cc',
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
			'render/canvas.cc',
			'math.h',
			'math.cc',
			'bezier.h',
			'bezier.cc',
			'media/media.h',
			'media/media.cc',
			'app.inl',
			'pre-render.h',
			'pre-render.cc',
			'app.h',
			'app.cc',
			'display.h',
			'display.cc',
			'errno.h',
			'event.h',
			'event.cc',
			'keyboard.h',
			'keyboard.cc',
			'image-src.h',
			'image-src.cc',
			'value.h',
			'os/os.h',
			'os/os.cc',
			#
			# '_property.h',
			# '_property.cc',
			# '_text-rows.cc',
			# '_text-rows.h',
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
					'platforms/unix/unix-gl.h',
					'platforms/unix/unix-gl.cc',
					'platforms/android/android-app.cc',
					'platforms/android/android-keyboard.cc',
					# 'os/android/org/flare/Activity.java',
					# 'os/android/org/flare/API.java',
					# 'os/android/org/flare/IMEHelper.java',
					'os/android/android-api.h',
					'os/android/android-api.cc',
					'os/android/android-os.cc',
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
				'dependencies': [
					'deps/reachability/reachability.gyp:reachability',
				],
				'sources':[
					'platforms/apple/apple-app.h',
					'platforms/apple/apple-image-codec.mm',
					'platforms/apple/apple-keyboard.mm',
					'platforms/apple/apple-metal.mm',
					'platforms/apple/apple-render.mm',
					'platforms/apple/apple-render.h',
					'os/apple/apple-os.mm',
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
					'platforms/linux/linux-gl.h',
					'platforms/linux/linux-gl.cc',
					'platforms/linux/linux-app.cc',
					'platforms/linux/linux-keyboard.cc',
					'platforms/linux/linux-ime-helper.h',
					'platforms/linux/linux-ime-helper.cc',
					'os/linux/linux-is.mm',
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
			'media/media-codec_inl.h',
			'media/media-codec_inl.cc',
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
					'platforms/apple/apple-media-codec.mm',
					'platforms/apple/apple-pcm-player.mm',
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
