{
  'variables': {
    'use_system_libpng%': 0,
  },
  'conditions': [
    ['use_system_libpng==0', {
      'targets': [{
        'target_name': 'libpng',
        'product_name': 'png-2',
        'type': '<(library)',
        'dependencies': [
          '../../deps/node/deps/zlib/zlib.gyp:zlib'
        ],
        'include_dirs': [
          '../../deps/node/deps/zlib'
        ],
        'defines': [
          'CHROME_PNG_WRITE_SUPPORT',
          'PNG_USER_CONFIG',
        ],
        'direct_dependent_settings': {
          'include_dirs': [ '.' ],
          'defines': [
            'CHROME_PNG_WRITE_SUPPORT',
            'PNG_USER_CONFIG',
          ],
        },
        'sources': [
          'png.c',
          'png.h',
          'pngconf.h',
          'pngerror.c',
          'pnggccrd.c',
          'pngget.c',
          'pngmem.c',
          'pngpread.c',
          'pngread.c',
          'pngrio.c',
          'pngrtran.c',
          'pngrutil.c',
          'pngset.c',
          'pngtrans.c',
          'pngusr.h',
          'pngvcrd.c',
          'pngwio.c',
          'pngwrite.c',
          'pngwtran.c',
          'pngwutil.c',
        ],
      }]
    }, {
      'conditions': [
        ['sysroot!=""', {
          'variables': {
            'pkg-config': '../../build/linux/pkg-config-wrapper "<(sysroot)"',
          },
        }, {
          'variables': {
            'pkg-config': 'pkg-config'
          },
        }],
      ],
      'targets': [
        {
          'target_name': 'libpng',
          'type': 'none',
          'dependencies': [
            '../zlib/zlib.gyp:zlib',
          ],
          'direct_dependent_settings': {
            'cflags': [
              '<!@(<(pkg-config) --cflags libpng)',
            ],
            'defines': [
              'USE_SYSTEM_LIBPNG',
            ],
          },
          'link_settings': {
            'ldflags': [
              '<!@(<(pkg-config) --libs-only-L --libs-only-other libpng)',
            ],
            'libraries': [
              '<!@(<(pkg-config) --libs-only-l libpng)',
            ],
          },
        },
      ],
    }],
  ],
}
