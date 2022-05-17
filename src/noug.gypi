{
	'targets':[
	{
		'target_name': 'noug',
		'type': 'static_library', #<(output_type)
		'include_dirs': [
			'..',
			'../out',
			'../deps/freetype2/include',
			'../deps/tess2/Include',
			'../deps/tinyxml2',
		],
		'dependencies': [
			'noug-util',
			'deps/tess2/tess2.gyp:tess2', 
			'deps/freetype2/freetype2.gyp:ft2',
			'deps/tinyxml2/tinyxml2.gyp:tinyxml2',
		],
		'direct_dependent_settings': {
			'include_dirs': [ '..', '../out' ],
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
			'render/font/pool.h',
			'render/font/pool.cc',
			'render/font/typeface.h',
			'render/font/typeface.cc',
			'render/font/style.h',
			'render/font/metrics.h',
			'render/font/font.h',
			'render/font/font.cc',
			'render/render.h',
			'render/render.cc',
			'render/pixel.h',
			'render/pixel.cc',
			'render/source.h',
			'render/source.cc',
			'render/path.h',
			'render/path.cc',
			'render/scaner.h',
			'render/scaner.cc',
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
			'value.h',
			'os/os.h',
			'os/os.cc',
			'effect.h',
			'effect.cc',
			#
			# 'property.h',
			# 'property.cc',
			'text_rows.cc',
			'text_rows.h',
			'text_settings.cc',
			'text_settings.h',
			# 'action/action.inl',
			# 'action/action.h',
			# 'action/group.h',
			# 'action/keyframe.h',
			# 'action/action.cc',
			# 'action/center.cc',
			# 'action/group.cc',
			# 'action/keyframe.cc',
			# 'action/action_prop.h',
			# 'action/action_prop.cc',
			# 'css/css.inl',
			# 'css/css.h',
			# 'css/cls.cc',
			# 'css/prop.cc',
			# 'css/scope.cc',
			# 'css/sheets.cc',
			#
			# 'render/codec/codec.h',
			# 'render/codec/codec.cc',
			# 'render/codec/pvrtc.cc',
			# 'render/codec/tga.cc',
			#
		],
		'conditions': [
			['use_gl==1 or use_skia==0', { # use opengl
				'defines': [ 'N_ENABLE_GL=1' ],
				'sources': [
					'render/gl.h',
					'render/gl.cc',
				],
			}],
			['use_skia==1', { # use skia
				'defines': [ 'N_ENABLE_SKIA=1' ],
				'sources': [
					'render/skia/skia_canvas.h',
					'render/skia/skia_canvas.cc',
					'render/skia/skia_render.h',
					'render/skia/skia_render.cc',
					'render/skia/skia_source.cc',
				],
				'conditions': [
					['OS=="mac" and project=="xcode"', {
						'dependencies': [ 'out/skia.gyp:skia_gyp' ], # debug in xcode
					}, {
						'dependencies': [ 'skia' ],
					}],
					['use_gl==1', {
						'sources': [
							'render/skia/skia_gl.cc',
						],
					}],
				],
			}],
			['use_skia==0', { # use fastuidraw
				'defines': [ 'N_ENABLE_FASTUIDRAW=1' ],
			}],
			['os=="android"', {
				'sources': [
					'platforms/unix/unix_gl.h',
					'platforms/unix/unix_gl.cc',
					'platforms/android/android_app.cc',
					'platforms/android/android_keyboard.cc',
					# 'os/android/org/noug/Activity.java',
					# 'os/android/org/noug/API.java',
					# 'os/android/org/noug/IMEHelper.java',
					'os/android/android_api.h',
					'os/android/android_api.cc',
					'os/android/android_os.cc',
					# 'render/vulkan.h',
					# 'render/vulkan.cc',
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
					# 'deps/libgif/libgif.gyp:libgif', 
					# 'deps/libjpeg/libjpeg.gyp:libjpeg', 
					# 'deps/libpng/libpng.gyp:libpng',
					# 'deps/libwebp/libwebp.gyp:libwebp',
				],
				'sources': [
					# 'render/codec/gif.cc',
					# 'render/codec/jpeg.cc',
					# 'render/codec/png.cc',
					# 'render/codec/webp.cc',
				],
			}],
			['OS=="mac"', {
				'dependencies': [
					'deps/reachability/reachability.gyp:reachability',
				],
				'sources':[
					'platforms/apple/apple_app.h',
					'platforms/apple/apple_keyboard.mm',
					'platforms/apple/apple_render.mm',
					'platforms/apple/apple_render.h',
					'render/metal.h',
					'render/metal.mm',
					'render/skia/skia_metal.mm',
					'os/apple/apple_os.mm',
				],
				'link_settings': {
					'libraries': [
						# '$(SDKROOT)/usr/lib/libz.tbd',
						'$(SDKROOT)/System/Library/Frameworks/CoreGraphics.framework',
						'$(SDKROOT)/System/Library/Frameworks/QuartzCore.framework',
						'$(SDKROOT)/System/Library/Frameworks/MetalKit.framework',
						
					]
				},
			}],
			['os=="ios"', {
				'sources':[
					'platforms/ios/ios_app.mm',
					'platforms/ios/ios_ime_helper.h',
					'platforms/ios/ios_ime_helper.mm',
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
					'platforms/osx/osx_app.mm',
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
		],
		# end
	},
	{
		'target_name': 'noug-media',
		'type': 'static_library', #<(output_type)
		'dependencies': [
			'noug',
			'skia',
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