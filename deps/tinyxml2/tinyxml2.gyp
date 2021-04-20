{
  'targets': [
    {
      'target_name': 'tinyxml2',
      'type': '<(library)',
      'direct_dependent_settings': {
        'include_dirs': [ '.', ],
      },
      'sources': [ 
        'tinyxml2.h', 
        'tinyxml2.cpp'
      ]
    },
  ],
}