{
	'targets': [{
		'target_name': 'trial',
		'type': '<(output_type)',
		'dependencies': [ 'flare-util' ],
		'sources': [
			'jsx.h',
			'jsx.cc',
			'fs.h',
			'fs-search.cc',
		],
	}],
}
