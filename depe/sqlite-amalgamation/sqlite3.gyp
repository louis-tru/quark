{
  'variables': {
    'use_system_sqlite%': 0
  },
  'targets':[{
    'target_name': 'sqlite3',
    'conditions': [
    # or use_system_sqlite==1
      [ 'OS in "mac ios"', {
        'type': 'none',
        'link_settings': { 'libraries': [ '$(SDKROOT)/usr/lib/libsqlite3.tbd' ] },
      },'use_system_sqlite==1', {
        'link_settings': { 'libraries': [ '-lsqlite3' ] },
      }, {
        'type': '<(library)',
        'direct_dependent_settings': {
          'include_dirs': [ '.' ],
        },
        'sources': [
          'sqlite3.h',
          'sqlite3ext.h',
          'sqlite3.c',
        ],
      }],
    ]
    # end conditions
  }]
}