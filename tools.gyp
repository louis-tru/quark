{
  'includes': [
    'ngui/base/base.gypi',
  ],
  'conditions': [
  	['os != "ios" or project == "xcode"', {
  		'includes': [ 
        'tools/tools.gypi',
      ],
  	}]
  ],
}
