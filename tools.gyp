{
	'includes': [
		'nxutils/nxutils.gypi',
		'tools/default_target.gypi',
	],
	'conditions': [
		['os != "ios" or project == "xcode"', {
			'includes': [ 
				'tools/tools.gypi',
			],
		}]
	],
}
