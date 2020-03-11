{
	'includes': [
		'nxkit/nxkit.gypi',
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
