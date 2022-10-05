{
	'targets': [{
		'target_name': 'trial',
		'type': '<(output_type)',
		'dependencies': [ 'quark-util' ],
		'sources': [
			'jsx.h',
			'jsx.cc',
			'fs.h',
			'fs-search.cc',
		],
	}],
}
