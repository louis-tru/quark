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
			'../include',
			'../../out',
			'../../depe/freetype2/include',
			'../../depe/tess2/Include',
			'../../depe/tinyxml2',
		],
		'dependencies': [
			'ftr-util',
			'depe/tess2/tess2.gyp:tess2', 
			'depe/freetype2/freetype2.gyp:ft2',
			'depe/tinyxml2/tinyxml2.gyp:tinyxml2',
		],
		'direct_dependent_settings': {
			'include_dirs': [ '../include' ],
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
			'../include/ftr/views2/div.h',
			'../include/ftr/views2/indep.h',
			'../include/ftr/views2/image.h',
			'../include/ftr/views2/label.h',
			'../include/ftr/views2/layout.h',
			'../include/ftr/views2/box.h',
			'../include/ftr/views2/view.h',
			'../include/ftr/views2/root.h',
			'../include/ftr/views2/sprite.h',
			'../include/ftr/views2/scroll.h',
			'../include/ftr/views2/span.h',
			'../include/ftr/views2/hybrid.h',
			'../include/ftr/views2/text-node.h',
			'../include/ftr/views2/text.h',
			'../include/ftr/views2/input.h',
			'../include/ftr/views2/textarea.h',
			'../include/ftr/views2/video.h',
			'../include/ftr/views2/box-shadow.h',
			'../include/ftr/views2/limit.h',
			'../include/ftr/views2/limit-indep.h',
			#
			'../include/ftr/math/bezier.h',
			'../include/ftr/math/math.h',
			'../include/ftr/display-port.h',
			'../include/ftr/event.h',
			'../include/ftr/codec/codec.h',
			'../include/ftr/codec/data.h',
			'../include/ftr/codec/classs.h',
			'../include/ftr/codec/name.h',
			'../include/ftr/codec/pseudo.h',
			'../include/ftr/codec/scope.h',
			'../include/ftr/codec/sheets.h',
			'../include/ftr/text-font.h',
			'../include/ftr/pre-render.h',
			'../include/ftr/draw/draw.h',
			'../include/ftr/draw/gl.h',
			'../include/ftr/texture.h',
			'../include/ftr/value.h',
			'../include/ftr/action/action.h',
			'../include/ftr/action/center.h',
			'../include/ftr/action/group.h',
			'../include/ftr/action/keyframe.h',
			'../include/ftr/action/sequence.h',
			'../include/ftr/action/spawn.h',
			'../include/ftr/app.h',
			'../include/ftr/font/font.h',
			'../include/ftr/keyboard.h',
			'../include/ftr/property.h',
			'../include/ftr/background.h',
			'../include/ftr/media/media.h',

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
			'font/font.h',
			'action/action.cc.inl',
			'action/action.cc',
			'app.cc',
			'app.h',
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
			'pre-render.cc',
			'math/math.cc',
			'text-rows.cc',
			'text-rows.h',
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
			'property.cc',
			'background.cc',
			'render-looper.h',
			'render-looper.cc',
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
