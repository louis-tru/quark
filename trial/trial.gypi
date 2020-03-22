{
	'targets': [{
		'target_name': 'trial',
		'type': '<(output_type)',
		'dependencies': [ 'nxkit' ],
		'sources': [
			'jsx.h',
			'jsx.cc',
			'fs.h',
			'fs-search.cc',
		],
	}],
}
