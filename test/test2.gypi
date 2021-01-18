{
	'targets': [{
		'target_name': 'test2',
		'type': 'executable',
		'include_dirs': [
			'../src/include',
		],
		'sources': [
			'test2.cc',
			'test2-thread.cc',
			'test2-x11.cc',
			'test2-xim.cc',
			'test2-alsa.cc',
			'test2-alsa2.cc',
			'test2-xopen.cc',
			'test2-sys.cc',
			'test2-str.cc',
			'../src/ftr/util/log.cc',
			'../src/ftr/util/str.cc',
			'../src/ftr/util/object.cc',
			'../src/ftr/util/hash.cc',
			'../src/ftr/util/codec.cc',
		],
		'mac_bundle': 1,
		'xcode_settings': {
			'OTHER_LDFLAGS': '-all_load',
		},
		'conditions': [
			['os in "ios osx"', {
				'sources': [
					'test2-opengl.mm',
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
