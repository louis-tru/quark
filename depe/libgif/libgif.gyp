{
  'targets': [
    {
      'target_name': 'libgif',
      'type': '<(library)',
      'direct_dependent_settings': {
        'include_dirs': [ 'lib' ],
      },
      'include_dirs': [ 'lib' ],
      'sources': [ 
        'lib/dgif_lib.c',
        'lib/egif_lib.c',
        'lib/gifalloc.c',
        'lib/gif_err.c',
        'lib/gif_font.c',
        'lib/gif_hash.c',
        'lib/gif_hash.h',
        'lib/gif_lib.h',
        'lib/gif_lib_private.h',
        'lib/openbsd-reallocarray.c',
        'lib/quantize.c',
      ],
      'conditions': [
        ['OS=="android"', {
          'defines': [ 'SIZE_MAX=4294967295U' ],
        }]
      ],
    },
  ],
}