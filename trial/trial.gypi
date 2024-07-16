{
	'targets': [{
		'target_name': 'trial',
		'type': 'static_library', #<(output_type)
		'dependencies': [ 'quark-util' ],
		'sources': [
			'jsx.h',
			'jsx.cc',
			'fs.h',
			'fs-search.cc',
			# 'raster.h',
			# 'raster.cc',
		],
	}],
}
