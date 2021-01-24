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
		'type': '<(output_type)',
		'include_dirs': [
			'..',
			'../out',
			'../depe/freetype2/include',
			'../depe/tess2/Include',
			'../depe/tinyxml2',
		],
		'dependencies': [
			'ftr-util',
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
			#
			'app.h',
			'text-font.h',
			'font/font.h',
			'math/bezier.h',
			'math/math.h',
			'display-port.h',
			'event.h',
			'codec/codec.h',
			'codec/data.h',
			'codec/classs.h',
			'codec/name.h',
			'codec/pseudo.h',
			'codec/scope.h',
			'codec/sheets.h',
			'draw/draw.h',
			'draw/gl.h',
			'texture.h',
			'value.h',
			'action/action.h',
			'action/center.h',
			'action/group.h',
			'action/keyframe.h',
			'action/sequence.h',
			'action/spawn.h',
			'keyboard.h',
			
			'background.h',
			'media/media.h',

			# views
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
			'_font/font.h',
			'action/action.cc.inl',
			'action/action.cc',
			'app.cc',
			'math/bezier.cc',
			'event.cc',
			'display-port.cc',
			'font/font.cc',
			'font/font.cc.all.inl',
			'font/font.cc.init.inl',
			'font/font.cc.levels.inl',
			'codec/codec.cc',
			'codec/codec-tga.cc',
			'codec/codec-pvrtc.cc',
			'math/math.cc',
			'draw/draw.cc',
			'draw/gl.cc',
			'draw/gl-draw.cc',
			'draw/gl-texture.cc',
			'draw/gl-font.cc',
			'text-font.cc',
			'texture.cc',
			'value.cc',
			'keyboard.cc',
			'css/css.cc.inl',
			'css/css.cc',
			'background.cc',
			'media/media.cc',
		],
		'conditions': [
			['os=="android"', {
				'sources': [
					'platforms/linux-gl.h',
					'platforms/linux-gl.cc',
					'platforms/android-app.cc',
					'platforms/android-keyboard.cc',
					'platforms/android-sys.cc',
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
					'depe/libgif/libgif.gyp:libgif', 
					'depe/libjpeg/libjpeg.gyp:libjpeg', 
					'depe/libpng/libpng.gyp:libpng',
					'depe/libwebp/libwebp.gyp:libwebp',
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
					'platforms/ios-ime-helper.h',
					'platforms/ios-ime-helper.mm',
					'platforms/ios-gl.h',
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
					'platforms/osx-gl.h',
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
		'target_name': 'ftr-media',
		'type': '<(output_type)',
		'dependencies': [
			'ftr',
			'depe/ffmpeg/ffmpeg.gyp:ffmpeg',
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
