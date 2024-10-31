{
	'targets':[
	{ # quark
		'target_name': 'quark',
		'type': 'static_library', #<(output_type)
		'include_dirs': [
			'../out',
			'../deps/freetype/include',
			'../deps/libtess2/Include',
			'../deps/tinyxml2',
		],
		'dependencies': [
			'quark-util', # util
			'deps/libtess2/libtess2.gyp:libtess2',
			'deps/freetype/freetype.gyp:freetype',
			'deps/tinyxml2/tinyxml2.gyp:tinyxml2',
			'deps/libgif/libgif.gyp:libgif',
		],
		'direct_dependent_settings': {
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
			'ui/app.h', # ui
			'ui/app.cc',
			'ui/screen.h',
			'ui/screen.cc',
			'ui/event.h',
			'ui/event.cc',
			'ui/pre_render.h',
			'ui/pre_render.cc',
			'ui/window.h',
			'ui/window.cc',
			'ui/keyboard.h',
			'ui/keyboard.cc',
			'ui/types.h',
			'ui/view_prop.h',
			'ui/view_prop.cc',
			'ui/filter.h',
			'ui/filter.cc',
			'ui/layer.h',
			'ui/layer.cc',
			'ui/draw.h',
			'ui/draw.cc',
			'ui/css/css.h', # css
			'ui/css/css_sheets.cc',
			'ui/css/css_class.cc',
			'ui/css/css_prop.cc',
			'ui/action/action.h', # action
			'ui/action/action.cc',
			'ui/action/center.cc',
			'ui/action/group.cc',
			'ui/action/keyframe.cc',
			'ui/action/keyframe.h',
			'ui/text/text_blob.h', # ui text
			'ui/text/text_blob.cc',
			'ui/text/text_input.h',
			'ui/text/text_lines.h',
			'ui/text/text_lines.cc',
			'ui/text/text_opts.cc',
			'ui/text/text_opts.h',
			'ui/view/box.h', # ui view
			'ui/view/box.cc',
			'ui/view/free.h',
			'ui/view/free.cc',
			'ui/view/flex.h',
			'ui/view/flex.cc',
			'ui/view/flow.h',
			'ui/view/flow.cc',
			'ui/view/image.h',
			'ui/view/image.cc',
			'ui/view/input.h',
			'ui/view/input.cc',
			'ui/view/textarea.h',
			'ui/view/textarea.cc',
			'ui/view/label.h',
			'ui/view/label.cc',
			'ui/view/root.h',
			'ui/view/root.cc',
			'ui/view/scroll.h',
			'ui/view/scroll.cc',
			'ui/view/text.h',
			'ui/view/text.cc',
			'ui/view/button.h',
			'ui/view/button.cc',
			'ui/view/view.h',
			'ui/view/view.cc',
			'ui/view/matrix.h',
			'ui/view/matrix.cc',
			'render/codec/codec.h', # render
			'render/codec/codec.cc',
			'render/codec/tga.cc',
			'render/codec/gif.cc',
			'render/codec/pvrtc.cc',
			'render/font/font.h',
			'render/font/font.cc',
			'render/font/metrics.h',
			'render/font/pool.h',
			'render/font/pool.cc',
			'render/font/style.h',
			'render/font/style.cc',
			'render/font/typeface.h',
			'render/font/typeface.cc',
			'render/font/util.h',
			'render/blend.h',
			'render/canvas.h',
			'render/canvas.cc',
			'render/paint.h',
			'render/paint.cc',
			'render/path.h',
			'render/path.cc',
			'render/pathv_cache.h',
			'render/pathv_cache.cc',
			'render/stroke.cc',
			'render/pixel.h',
			'render/pixel.cc',
			'render/render.h',
			'render/render.cc',
			'render/bezier.h',
			'render/bezier.cc',
			'render/math.h',
			'render/math.cc',
			'render/source.h',
			'render/source.cc',
			'os/os.h', # os
			'os/os.cc',
			'errno.h',
			'version.h',
		],
		'conditions': [
			['use_gl==1', { # use opengl
				'defines': [ 'Qk_ENABLE_GL=1' ],
				'sources': [
					'render/gl/gl_canvas.h',
					'render/gl/gl_canvas.cc',
					'render/gl/gl_cmd.h',
					'render/gl/gl_cmd.cc',
					'render/gl/gl_render.h',
					'render/gl/gl_render.cc',
					'render/gl/gl_shader.h',
					'render/gl/gl_shader.cc',
				],
			}],
			['OS=="mac" and use_gl==0', { # use metal
				'defines': [ 'Qk_ENABLE_METAL=1' ],
				'sources': [
					'render/metal/metal_canvas.h',
					'render/metal/metal_canvas.mm',
					'render/metal/metal_render.h',
					'render/metal/metal_render.mm',
				],
			}],
			['OS!="mac"', { # not mac mac
				'dependencies': [
					'deps/libjpeg/libjpeg.gyp:libjpeg',
					'deps/libpng/libpng.gyp:libpng',
					'deps/libwebp/libwebp.gyp:libwebp',
				],
				'sources': [
					'render/codec/jpeg.cc',
					'render/codec/png.cc',
					'render/codec/webp.cc',
				],
			}],
			['os=="android"', {
				'sources': [
					'platforms/linux/linux_gl.h',
					'platforms/linux/linux_gl.cc',
					'platforms/android/android_app.cc',
					'platforms/android/android_keyboard.cc',
					# 'platforms/android/org/quark/Activity.java',
					# 'platforms/android/org/quark/API.java',
					# 'platforms/android/org/quark/IMEHelper.java',
					'platforms/android/android_api.h',
					'platforms/android/android_api.cc',
					'platforms/android/android_device.cc',
				],
				'link_settings': {
					'libraries': [
						'-lGLESv3',
						'-lEGL',
						'-lz', '-landroid', '-llog',
					],
				},
			}],
			['OS=="mac"', { # mac mac, osx ios
				'dependencies': [
					'deps/reachability/reachability.gyp:reachability',
				],
				'sources':[
					'os/os_mac.mm',
					'platforms/mac/mac_app.h',
					'platforms/mac/mac_app.mm',
					'platforms/mac/mac_keyboard.mm',
					'render/codec/codec_mac.mm',
					'render/render_mac.h',
					'render/render_mac.mm',
					'render/font/util_mac.h',
					'render/font/ct/ct_pool.cc',
					'render/font/ct/ct_typeface.cc',
					'render/font/ct/ct_typeface.h',
					'render/font/ct/ct_util.cc',
					'render/font/ct/ct_util.h',
				],
				'link_settings': {
					'libraries': [
						'$(SDKROOT)/System/Library/Frameworks/CoreGraphics.framework',
						'$(SDKROOT)/System/Library/Frameworks/QuartzCore.framework',
						'$(SDKROOT)/System/Library/Frameworks/MetalKit.framework',
					]
				},
			}],
			['os=="ios"', {
				'sources': [
					'platforms/mac/ios_app.mm',
					'platforms/mac/ios_screen.mm',
					'platforms/mac/ios_ime_helper.mm',
					'platforms/mac/ios_main.mm',
					'platforms/mac/ios_window.mm',
					'render/render_ios.mm',
				],
				'link_settings': {
					'libraries': [
						'$(SDKROOT)/System/Library/Frameworks/OpenGLES.framework',
						'$(SDKROOT)/System/Library/Frameworks/UIKit.framework',
						'$(SDKROOT)/System/Library/Frameworks/MessageUI.framework',
						'$(SDKROOT)/System/Library/Frameworks/CoreText.framework',
					]
				},
			}],
			['os=="osx"', {
				'sources': [
					'platforms/mac/osx_app.mm',
					'platforms/mac/osx_screen.mm',
					'platforms/mac/osx_ime_helper.mm',
					'platforms/mac/osx_main.mm',
					'platforms/mac/osx_window.mm',
					'render/render_osx.mm',
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
					'platforms/linux/linux_device.cc',
					'platforms/linux/linux_gl.h',
					'platforms/linux/linux_gl.cc',
					'platforms/linux/linux_app.cc',
					'platforms/linux/linux_keyboard.cc',
					'platforms/linux/linux_ime_helper.h',
					'platforms/linux/linux_ime_helper.cc',
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
					'../tools/gen_font_natives.js',
					'../tools/DejaVuSerif.ttf',
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
			{
				'action_name': 'gen_glsl_natives',
				'inputs': [
					'../tools/gen_glsl_natives.js',
					'render/gl/glsl/_util.glsl',
					'render/gl/glsl/_blur.glsl',
					'render/gl/glsl/_image.glsl',
					'render/gl/glsl/clear.glsl',
					'render/gl/glsl/clip_test.glsl',
					'render/gl/glsl/clip_aa.glsl',
					'render/gl/glsl/colors.glsl',
					'render/gl/glsl/color.glsl',
					'render/gl/glsl/color_linear.glsl',
					'render/gl/glsl/color_radial.glsl',
					'render/gl/glsl/color_rrect_blur.glsl',
					'render/gl/glsl/image.glsl',
					'render/gl/glsl/image_mask.glsl',
					'render/gl/glsl/image_yuv.glsl',
					'render/gl/glsl/vport_cp.glsl',
					'render/gl/glsl/blur.glsl',
					'render/gl/glsl/blur3.glsl',
					'render/gl/glsl/blur7.glsl',
					'render/gl/glsl/blur13.glsl',
					'render/gl/glsl/blur19.glsl',
				],
				'outputs': [
					'render/gl/glsl_shaders.h',
					'render/gl/glsl_shaders.cc',
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
	{ # media
		'target_name': 'quark-media',
		'type': 'static_library', #<(output_type)
		'dependencies': [
			'quark',
			'deps/ffmpeg/ffmpeg.gyp:ffmpeg',
		],
		'include_dirs': [
			'../deps/libuv/include',
		],
		'sources': [
			'media/player.h',
			'media/player.cc',
			'media/media_codec_software.cc',
			'media/media_codec.cc',
			'media/media.cc',
			'media/media_inl.h',
			'media/media_source.cc',
			'media/media.h',
			'media/pcm_player.h',
			'ui/view/video.h',
			'ui/view/video.cc',
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
					'platforms/mac/mac_media_codec.mm',
					'platforms/mac/mac_pcm_player.mm',
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
	}],
}
