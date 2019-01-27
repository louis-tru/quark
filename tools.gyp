{
	'includes': [
		'shark/utils/utils.gypi',
	],
	'conditions': [
		['os != "ios" or project == "xcode"', {
			'includes': [ 
				'tools/tools.gypi',
			],
		}]
	],
}
