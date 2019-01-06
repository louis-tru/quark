{
  'variables': {
    'use_system_libjpeg%': 0,
    'use_system_yasm%': 1,
    'shared_generated_dir': '<(SHARED_INTERMEDIATE_DIR)/depe/libjpeg-turbo',
    'conditions': [
      [ 'OS=="win"', {
        'object_suffix': 'obj',
      }, {
        'object_suffix': 'o',
      }],
    ],
  },
  'conditions': [
    [ 'use_system_libjpeg==0', {
      'targets': [
        {
          'target_name': 'libjpeg',
          'product_name': 'jpeg2',
          'type': '<(library)',
          'include_dirs': [ '.', '../..' ],
          'defines': [
            'WITH_SIMD', 'MOTION_JPEG_SUPPORTED',
          ],
          'sources': [
            # 'bmp.c',
            # 'bmp.h',
            # 'rdppm.c',
            # 'rdbmp.c',
            # 'rdgif.c',
            # 'rdrle.c’,
            # 'rdtarga.c',
            'cderror.h',
            'cdjpeg.c',
            'cdjpeg.h',
            'jaricom.c',
            'jcapimin.c',
            'jcapistd.c',
            'jcarith.c',
            'jccoefct.c',
            'jccolor.c',
            'jcdctmgr.c',
            'jchuff.c',
            'jchuff.h',
            'jcinit.c',
            'jcmainct.c',
            'jcmarker.c',
            'jcmaster.c',
            'jcomapi.c',
            'jconfig.h',
            'jconfigint.h',
            'jcparam.c',
            'jcphuff.c',
            'jcprepct.c',
            'jcsample.c',
            'jctrans.c',
            'jdapimin.c',
            'jdapistd.c',
            'jdarith.c',
            'jdatadst.c',
            'jdatadst-tj.c',
            'jdatasrc.c',
            'jdatasrc-tj.c',
            'jdcoefct.c',
            'jdcoefct.h',
            'jdcolor.c',
            'jdct.h',
            'jddctmgr.c',
            'jdhuff.c',
            'jdhuff.h',
            'jdinput.c',
            'jdmainct.c',
            'jdmainct.h',
            'jdmarker.c',
            'jdmaster.c',
            'jdmaster.h',
            'jdmerge.c',
            'jdphuff.c',
            'jdpostct.c',
            'jdsample.c',
            'jdsample.h',
            'jdtrans.c',
            'jerror.c',
            'jerror.h',
            'jfdctflt.c',
            'jfdctfst.c',
            'jfdctint.c',
            'jidctflt.c',
            'jidctfst.c',
            'jidctint.c',
            'jidctred.c',
            'jinclude.h',
            'jmemmgr.c',
            'jmemnobs.c',
            'jmemsys.h',
            'jmorecfg.h',
            'jpegcomp.h',
            'jpegint.h',
            'jpeglib.h',
            'jpeg_nbits_table.h',
            'jquant1.c',
            'jquant2.c',
            'jsimddct.h',
            'jsimd.h',
            'jutils.c',
            'jversion.h',
            'transupp.c',
            'transupp.h',
            'turbojpeg.c',
            'turbojpeg.h',
          ],
          'direct_dependent_settings': {
            'include_dirs': [ '.' ],
          },
          'msvs_disabled_warnings': [4018, 4101],
          'conditions': [
            # Add target-specific source files.
            [ 'target_arch=="ia32"', {
              'sources': [
                'simd/jsimd.h',
                'simd/jsimd_i386.c',
                'simd/jsimdcfg.inc',
                'simd/jsimdext.inc',
                'simd/jcolsamp.inc',
                'simd/jdct.inc',
                'simd/jpeg_nbits_table.inc',
                'simd/jccolor-sse2.asm',
                'simd/jcgray-sse2.asm',
                'simd/jchuff-sse2.asm',
                'simd/jcsample-sse2.asm',
                'simd/jdcolor-sse2.asm',
                'simd/jdmerge-sse2.asm',
                'simd/jdsample-sse2.asm',
                'simd/jfdctflt-sse.asm',
                'simd/jfdctfst-sse2.asm',
                'simd/jfdctint-sse2.asm',
                'simd/jidctflt-sse2.asm',
                'simd/jidctfst-sse2.asm',
                'simd/jidctint-sse2.asm',
                'simd/jidctred-sse2.asm',
                'simd/jquantf-sse2.asm',
                'simd/jquanti-sse2.asm',
                'simd/jccolor-mmx.asm',
                'simd/jcgray-mmx.asm',
                'simd/jcsample-mmx.asm',
                'simd/jdcolor-mmx.asm',
                'simd/jdmerge-mmx.asm',
                'simd/jdsample-mmx.asm',
                'simd/jfdctfst-mmx.asm',
                'simd/jfdctint-mmx.asm',
                'simd/jidctfst-mmx.asm',
                'simd/jidctint-mmx.asm',
                'simd/jidctred-mmx.asm',
                'simd/jquant-mmx.asm',
                'simd/jidctflt-sse.asm',
                'simd/jquant-sse.asm',
                'simd/jsimdcpu.asm',
                'simd/jfdctflt-3dn.asm',
                'simd/jidctflt-3dn.asm',
                'simd/jquant-3dn.asm',
              ],
            }],
            [ 'target_arch=="x64"', {
              'sources': [
                'simd/jsimd.h',
                'simd/jsimd_x86_64.c',
                'simd/jsimdcfg.inc',
                'simd/jsimdext.inc',
                'simd/jcolsamp.inc',
                'simd/jdct.inc',
                'simd/jpeg_nbits_table.inc',
                'simd/jcgray-sse2-64.asm',
                'simd/jccolor-sse2-64.asm',
                'simd/jchuff-sse2-64.asm',
                'simd/jcsample-sse2-64.asm',
                'simd/jdcolor-sse2-64.asm',
                'simd/jdmerge-sse2-64.asm',
                'simd/jdsample-sse2-64.asm',
                'simd/jfdctflt-sse-64.asm',
                'simd/jfdctfst-sse2-64.asm',
                'simd/jfdctint-sse2-64.asm',
                'simd/jidctflt-sse2-64.asm',
                'simd/jidctfst-sse2-64.asm',
                'simd/jidctint-sse2-64.asm',
                'simd/jidctred-sse2-64.asm',
                'simd/jquantf-sse2-64.asm',
                'simd/jquanti-sse2-64.asm',
              ],
            }],
            # The ARM SIMD implementation can be used for devices that support
            # the NEON instruction set. This is done dynamically by probing CPU
            # features at runtime, so always compile it for ARMv7-A devices.
            [ 'target_arch=="arm"', {
              'conditions': [
                [ 'armv7==1 or arm_neon==1', {
                  'sources': [
                    'simd/jsimd_arm.c',
                    'simd/jsimd_arm_neon.S',
                  ],
                },{
                  'sources': [
                    'jsimd_none.c',
                  ]
                }]
              ],
            }],
            [ 'target_arch=="arm64"', {
              'sources': [
                'simd/jsimd_arm64.c',
                'simd/jsimd_arm64_neon.S',
              ]
            }],
            [ 'target_arch=="mips" or target_arch=="mips64"', {
              'sources': [
                'simd/jsimd_mips.c',
                'simd/jsimd_mips_dspr2_asm.h',
                'simd/jsimd_mips_dspr2.S',
              ]
            }],
            [ 'target_arch=="ppc"', {
              'sources': [
                'simd/jcsample.h',
                'simd/jsimd_altivec.h',
                'simd/jccolext-altivec.c',
                'simd/jccolor-altivec.c',
                'simd/jcgray-altivec.c',
                'simd/jcsample-altivec.c',
                'simd/jdcolor-altivec.c',
                'simd/jdmerge-altivec.c',
                'simd/jdsample-altivec.c',
                'simd/jfdctfst-altivec.c',
                'simd/jfdctint-altivec.c',
                'simd/jidctfst-altivec.c',
                'simd/jidctint-altivec.c',
                'simd/jquanti-altivec.c',
                'simd/jsimd_powerpc.c',
              ],
              'cflags': [ '-maltivec' ],
            }],
            # Build rules for an asm file.
            # On Windows, we use the precompiled yasm binary. On Linux, we build
            # our patched yasm and use it except when use_system_yasm is 1. On
            # Mac, we always build our patched yasm and use it because of
            # <http://www.tortall.net/projects/yasm/ticket/236>.
            [ 'OS=="win"', {
              'variables': {
                # 'yasm_path': '../yasm/binaries/win/yasm<(EXECUTABLE_SUFFIX)',
                'yasm_path': 'yasm<(EXECUTABLE_SUFFIX)',
                'yasm_format': '-fwin32',
                'yasm_flags': [
                  '-DWIN32',
                  '-DMSVC',
                  '-Iwin/'
                ],
              },
            }],
            [ 'os=="osx"', {
              'conditions': [
                [ 'use_system_yasm==1', {
                  'variables': { 'yasm_path': '<!(which yasm)' }
                }, {
                  'dependencies': [ '../yasm/yasm.gyp:yasm#host' ],
                  'variables': { 'yasm_path': '<(PRODUCT_DIR)/yasm' }
                }],
                [ 'target_arch=="ia32"', {
                  'variables': {
                    'yasm_format': '-fmacho',
                    'yasm_flag': '-D__X86__',
                    'yasm_flags': [
                      '-D__x86__',
                      '-DMACHO',
                      '-Imac/'
                    ],
                  }
                }, {
                  'variables': {
                    'yasm_format': '-fmacho64',
                    'yasm_flag': '-D__x86_64__',
                    'yasm_flags': [
                      '-D__x86_64__',
                      '-DMACHO',
                      '-Imac/'
                    ],
                  }
                }],
              ],
            }],
            [ '(OS=="linux" or OS=="android") and (target_arch=="ia32" or target_arch=="x64")', {
              'conditions': [
                [ 'use_system_yasm==1', {
                  'variables': { 'yasm_path': '<!(which yasm)' }
                }, {
                  'dependencies': [ '../yasm/yasm.gyp:yasm#host' ],
                  'variables': { 'yasm_path': '<(PRODUCT_DIR)/yasm' }
                }],
                [ 'target_arch=="ia32"', {
                  'variables': {
                    'yasm_format': '-felf',
                    'yasm_flag': '-D__X86__',
                    'yasm_flags': [
                      '-D__x86__',
                      '-DELF',
                      '-Ilinux/'
                    ],
                  }
                }, {
                  'variables': {
                    'yasm_format': '-felf64',
                    'yasm_flag': '-D__x86_64__',
                    'yasm_flags': [
                      '-D__x86_64__',
                      '-DELF',
                      '-Ilinux/'
                    ],
                  }
                }],
              ],
            }],
          ],
          'rules': [
            {
              'rule_name': 'assemble',
              'extension': 'asm',
              'conditions': [
                [ 'target_arch=="ia32" or target_arch=="x64" ', {
                  'inputs': [ '<(yasm_path)', ],
                  'outputs': [
                    '<(shared_generated_dir)/<(RULE_INPUT_ROOT).<(object_suffix)',
                  ],
                  'action': [
                    '<(yasm_path)',
                    '<(yasm_format)',
                    '<@(yasm_flags)',
                    '-DRGBX_FILLER_0XFF',
                    '-DSTRICT_MEMORY_ACCESS',
                    '-Isimd/',
                    '-o', '<(shared_generated_dir)/<(RULE_INPUT_ROOT).<(object_suffix)',
                    '<(RULE_INPUT_PATH)',
                  ],
                  'process_outputs_as_sources': 1,
                  'message': 'Building <(RULE_INPUT_ROOT).<(object_suffix)',
                }],
              ]
            },
          ],
        },
      ],
    }, { # else: use_system_libjpeg != 0
      'targets': [
        {
          'target_name': 'libjpeg',
          'type': 'none',
          'direct_dependent_settings': {
            'defines': [ 'USE_SYSTEM_LIBJPEG' ],
          },
          'link_settings': {
            'libraries': [ '-ljpeg' ],
          },
        },
      ],
    }],
  ],
}