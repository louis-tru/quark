{
	'variables': {
		'gui_glsl_files': [
			'gl/glsl/_version.glsl',
			'gl/glsl/_util.glsl',
			'gl/glsl/_box.glsl',
			'gl/glsl/_box-radius.glsl',
			'gl/glsl/sprite.glsl',
			'gl/glsl/box-image.glsl',
			'gl/glsl/box-yuv420p-image.glsl',
			'gl/glsl/box-yuv420sp-image.glsl',
			'gl/glsl/box-border.glsl',
			'gl/glsl/box-border-radius.glsl',
			'gl/glsl/box-background-image.glsl',
			'gl/glsl/box-color.glsl',
			'gl/glsl/box-shadow.glsl',
			'gl/glsl/text-box-color.glsl',
			'gl/glsl/text-texture.glsl',
			'gl/glsl/text-vertex.glsl',
			'gl/glsl/gen-texture.glsl',
		],
		'gui_default_font_files': [
			'font/langou.ttf',
			'font/iconfont.ttf',
		],
	},
	'targets':[
	{
		'target_name': 'ftr',
		'type': 'static_library', #<(output_type)
		'include_dirs': [
			'..',
			'../out',
			'../deps/freetype2/include',
			'../deps/tess2/Include',
			'../deps/tinyxml2',
		],
		'dependencies': [
			'ftr-util',
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
			# views
			'views2/div.h',
			'views2/indep.h',
			'views2/image.h',
			'views2/label.h',
			'views2/layout.h',
			'views2/box.h',
			'views2/view.h',
			'views2/root.h',
			'views2/sprite.h',
			'views2/scroll.h',
			'views2/span.h',
			'views2/hybrid.h',
			'views2/text-node.h',
			'views2/text.h',
			'views2/input.h',
			'views2/textarea.h',
			'views2/video.h',
			'views2/box-shadow.h',
			'views2/limit.h',
			'views2/limit-indep.h',
			'views2/div.cc',
			'views2/indep.cc',
			'views2/box-shadow.cc',
			'views2/limit.cc',
			'views2/limit-indep.cc',
			'views2/image.cc',
			'views2/label.cc',
			'views2/layout.cc',
			'views2/box.cc',
			'views2/view.cc',
			'views2/root.cc',
			'views2/sprite.cc',
			'views2/scroll.cc',
			'views2/span.cc',
			'views2/hybrid.cc',
			'views2/text-node.cc',
			'views2/text.cc',
			'views2/input.cc',
			'views2/textarea.cc',
			'views2/panel.h',
			'views2/panel.cc',
			'views2/button.h',
			'views2/button.cc',
			#
			'action/_action.h',
			'action/action.h',
			'action/group.h',
			'action/keyframe.h',
			'action/action.cc',
			'action/center.cc',
			'action/group.cc',
			'action/keyframe.cc',
			'action/_property.h',
			'action/property.cc',
			'codec/codec.h',
			'codec/codec.cc',
			'codec/codec-tga.cc',
			'codec/codec-pvrtc.cc',
			'css/_css.h',
			'css/css.h',
			'css/cls.cc',
			'css/prop.cc',
			'css/scope.cc',
			'css/sheets.cc',
			'font/_font.h',
			'font/font.h',
			'font/pool.h',
			'font/defaults.cc',
			'font/family.cc',
			'font/font.cc',
			'font/glyph.cc',
			'font/levels.cc',
			'font/pool.cc',
			'gl/gl.h',
			'gl/gl.cc',
			'gl/gl-draw.cc',
			'gl/gl-texture.cc',
			'gl/gl-font.cc',
			'math/math.h',
			'math/math.cc',
			'math/bezier.h',
			'math/bezier.cc',
			'media/media.h',
			'media/media.cc',
			#
			'_app.h',
			'_pre-render.h',
			'_pre-render.cc',
			'_property.h',
			'_property.cc',
			'_render-looper.h',
			'_render-looper.cc',
			'_text-rows.cc',
			'_text-rows.h',
			'app.h',
			'app.cc',
			'background.h',
			'background.cc',
			'display-port.h',
			'display-port.cc',
			'draw.h',
			'draw.cc',
			'errno.h',
			'event.h',
			'event.cc',
			'keyboard.h',
			'keyboard.cc',
			'text-font.h',
			'text-font.cc',
			'texture.h',
			'texture.cc',
			'value.h',
			'value.cc',
		],
		'conditions': [
			['os=="android"', {
				'sources': [
					'platforms/_linux-gl.h',
					'platforms/linux-gl.cc',
					'platforms/android-app.cc',
					'platforms/android-keyboard.cc',
					# '../android/org/ftr/FtrActivity.java',
					# '../android/org/ftr/Android.java',
					# '../android/org/ftr/IMEHelper.java',
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
					'platforms/_mac-app.h',
					'platforms/mac-image-codec.mm',
					'platforms/mac-keyboard.mm',
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
					'platforms/ios-app.mm',
					'platforms/_ios-ime-helper.h',
					'platforms/ios-ime-helper.mm',
					'platforms/_ios-gl.h',
					'platforms/ios-gl.mm',
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
					'platforms/osx-app.mm',
					'platforms/_osx-gl.h',
					'platforms/osx-gl.mm',
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
				'action_name': 'gen_glsl_natives',
				'inputs': [
					'../tools/gen-glsl-natives.js',
					'<@(gui_glsl_files)',
				],
				'outputs': [
					'../out/native-glsl.h',
					'../out/native-glsl.cc',
				],
				'action': [
					'<(node)',
					'<@(_inputs)',
					'<@(_outputs)',
					'',
				],
				'process_outputs_as_sources': 1,
			},
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
		'target_name': 'ftr-media',
		'type': 'static_library', #<(output_type)
		'dependencies': [
			'ftr',
			'deps/ffmpeg/ffmpeg.gyp:ffmpeg',
		],
		'sources': [
			'media/pcm.h',
			'media/audio-player.h',
			'media/audio-player.cc',
			'views2/video.cc',
			'media/media-codec.h',
			'media/media-codec.cc',
			'media/_media-codec.h',
			'media/_media-codec.cc',
			'media/media-codec-software.cc',
			'media/media-init.cc',
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
					'platforms/mac-media-codec.mm',
					'platforms/mac-pcm-player.mm',
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
	]
}
