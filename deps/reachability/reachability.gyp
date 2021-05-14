{
  'targets': [
    {
      'target_name': 'reachability',
      'type': '<(library)',
      'direct_dependent_settings': {
        'include_dirs': [ '.' ],
      },
      'link_settings': { 
        'libraries': [ '$(SDKROOT)/System/Library/Frameworks/SystemConfiguration.framework' ],
      },
      'xcode_settings': {
        'CLANG_ENABLE_OBJC_ARC': 'NO',
      },
      'sources': [
        'reachability.h',
        'reachability.m',
      ],
    },
  ],
}
