{
  'targets': [
    {
      'target_name': 'tess2',
      'type': '<(library)',
      'direct_dependent_settings': {
        'include_dirs': [
          'Include',
          # 'Contrib',
        ],
      },
      'include_dirs': [
        'Include',
      ],
      'sources': [
        # inc
        # 'Contrib/nanosvg.h',
        'Include/tesselator.h',
        'Source/bucketalloc.h',
        'Source/dict.h',
        'Source/geom.h',
        'Source/mesh.h',
        'Source/priorityq.h',
        'Source/sweep.h',
        'Source/tess.h',
        # src
        # 'Contrib/nanosvg.c',
        'Source/bucketalloc.c',
        'Source/dict.c',
        'Source/geom.c',
        'Source/mesh.c',
        'Source/priorityq.c',
        'Source/sweep.c',
        'Source/tess.c',
      ]
    }
  ]  
}