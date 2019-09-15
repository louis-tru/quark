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
		'target_name': 'langou',
		'type': '<(output_type)',
		'include_dirs': [
			'..',
			'../out',
			'../depe/freetype2/include',
			'../depe/tess2/Include',
			'../depe/tinyxml2',
		],
		'dependencies': [
			'lutils',
			'depe/tess2/tess2.gyp:tess2', 
			'depe/freetype2/freetype2.gyp:ft2',
			'depe/tinyxml2/tinyxml2.gyp:tinyxml2',
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
			'div.h',
			'indep.h',
			'image.h',
			'bezier.h',
			'display-port.h',
			'event.h',
			'image-codec.h',
			'label.h',
			'layout.h',
			'box.h',
			'text-font.h',
			'pre-render.h',
			'mathe.h',
			'view.h',
			'draw.h',
			'root.h',
			'sprite.h',
			'scroll.h',
			'span.h',
			'hybrid.h',
			'text-node.h',
			'texture.h',
			'video.h',
			'value.h',
			'action.h',
			'action.cc.inl',
			'action.cc',
			'app.h',
			'app.cc',
			'app-1.h',
			'div.cc',
			'indep.cc',
			'box-shadow-1.h',
			'box-shadow.cc',
			'limit.h',
			'limit.cc',
			'limit-indep.h',
			'limit-indep.cc',
			'image.cc',
			'bezier.cc',
			'event.cc',
			'display-port.cc',
			'font.h',
			'font/font-1.h',
			'font/font.cc',
			'font/font.cc.all.inl',
			'font/font.cc.init.inl',
			'font/font.cc.levels.inl',
			'image/codec.cc',
			'image/codec-tga.cc',
			'image/codec-pvrtc.cc',
			'pre-render.cc',
			'mathe.cc',
			'label.cc',
			'layout.cc',
			'box.cc',
			'text-rows.cc',
			'text-rows.h',
			'view.cc',
			'draw.cc',
			'gl/gl.cc',
			'gl/gl.h',
			'gl/gl-draw.cc',
			'gl/gl-texture.cc',
			'gl/gl-font.cc',
			'root.cc',
			'sprite.cc',
			'scroll.cc',
			'span.cc',
			'hybrid.cc',
			'text-font.cc',
			'text-node.cc',
			'texture.cc',
			'value.cc',
			'panel.h',
			'panel.cc',
			'button.h',
			'button.cc',
			'keyboard.h',
			'keyboard.cc',
			'css.h',
			'css.cc.inl',
			'css.cc',
			'property.h',
			'property.cc',
			'text.h',
			'text.cc',
			'input.h',
			'input.cc',
			'textarea.h',
			'textarea.cc',
			'background.h',
			'background.cc',
			'render-looper.cc',
			'sys.h',
			'sys.cc',
			'sys.h',
			'sys.cc',
			'media.h',
			'media.cc',
		],
		'conditions': [
			['os=="android"', {
				'sources': [
					'platforms/linux-gl-1.h',
					'platforms/linux-gl.cc',
					'platforms/android-app.cc',
					'platforms/android-keyboard.cc',
					'platforms/android-sys.cc',
					'../android/com/langou/LangouActivity.java',
					'../android/com/langou/Android.java',
					'../android/com/langou/IMEHelper.java',
				],
				'link_settings': { 
					'libraries': [
						'-lGLESv3',
						'-lEGL',
					],
				},
			}],
			['OS!="mac"', {
				'dependencies': [
					'depe/libgif/libgif.gyp:libgif', 
					'depe/libjpeg/libjpeg.gyp:libjpeg', 
					'depe/libpng/libpng.gyp:libpng',
					'depe/libwebp/libwebp.gyp:libwebp',
				],
				'sources': [
					'image/codec-gif.cc',
					'image/codec-jpeg.cc',
					'image/codec-png.cc',
					'image/codec-webp.cc',
				],
				'link_settings': {
					'libraries': [
						# '-lz',
					]
				},
			}],
			['OS=="mac"', {
				'dependencies': [
					'depe/reachability/reachability.gyp:reachability',
				],
				'sources':[
					'platforms/mac-app.h',
					'platforms/mac-image-codec.mm',
					'platforms/mac-keyboard.mm',
					'platforms/mac-sys.mm',
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
					'platforms/ios-ime-helper-1.h',
					'platforms/ios-ime-helper.mm',
					'platforms/ios-gl-1.h',
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
					'platforms/osx-gl-1.h',
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
					'platforms/linux-app.cc',
					'platforms/linux-gl.cc',
					'platforms/linux-keyboard.cc',
					'platforms/linux-ime-helper.cc',
					'platforms/linux-sys.cc',
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
		'target_name': 'langou-media',
		'type': '<(output_type)',
		'dependencies': [
			'lutils',
			'langou',
			'depe/FFmpeg/FFmpeg.gyp:FFmpeg',
		],
		'sources': [
			'pcm-player.h',
			'audio-player.h',
			'audio-player.cc',
			'video.cc',
			'media-codec.h',
			'media-codec.cc',
			'media-codec-1.h',
			'media-codec-1.cc',
			'media-codec-software.cc',
			'media-init.cc',
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
