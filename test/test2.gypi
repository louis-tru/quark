{
	'targets': [{
		'target_name': 'test2',
		'type': 'executable',
		'sources': [
			'test2.cc',
			'test2-thread.cc',
			'test2-x11.cc',
			'test2-xim.cc',
			'test2-alsa.cc',
			'test2-alsa2.cc',
			#'test2-xopen.cc',
			'test2-xopen2.cc',
			'test2-sys.cc',
			'test2-opengl.mm',
		],
		# 'mac_bundle': 1,
		'xcode_settings': {
			# 'OTHER_LDFLAGS': '-all_load',
		},
		'conditions': [
			['os in "ios osx"', {
				'sources': [
					'test-<(os).plist',
					'Storyboard-<(os).storyboard',
				],
				'xcode_settings': {
					'INFOPLIST_FILE': '$(SRCROOT)/test/test-<(os).plist',
				},
			}],
			['os=="linux"', {
				'link_settings': { 
					'libraries': [ 
						'-lGLESv2', '-lEGL', '-lX11', '-lasound',
					],
				},
			}],
			['os=="osx"', {
				'link_settings': {
					'libraries': [
						'$(SDKROOT)/System/Library/Frameworks/AppKit.framework',
						'$(SDKROOT)/System/Library/Frameworks/OpenGL.framework',
						'$(SDKROOT)/System/Library/Frameworks/CoreVideo.framework',
					],
				},
			}],
		],
	}],
}
