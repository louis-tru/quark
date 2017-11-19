{
  'targets': [
    {
      'target_name': 'ft2',
      'type': '<(library)',
      'direct_dependent_settings': {
        'include_dirs': [ 'include' ],
      },
      'defines': [ 
        'FT2_BUILD_LIBRARY'
      ],
      'include_dirs':[ 'include' ],
      'cflags': [ '-ansi' ],
      'xcode_settings': {
        'GCC_C_LANGUAGE_STANDARD': 'ansi',
      },
      # 'defines!': [ 'DEBUG', ],
      'sources': [
        # inc
        'include/ft2build.h',
        'include/freetype/config/ftconfig.h',
        'include/freetype/config/ftheader.h',
        'include/freetype/config/ftmodule.h',
        'include/freetype/config/ftoption.h',
        'include/freetype/config/ftstdlib.h',
        'include/freetype/freetype.h',
        'include/freetype/ftadvanc.h',
        'include/freetype/ftautoh.h',
        'include/freetype/ftbbox.h',
        'include/freetype/ftbdf.h',
        'include/freetype/ftbitmap.h',
        'include/freetype/ftbzip2.h',
        'include/freetype/ftcache.h',
        'include/freetype/ftcffdrv.h',
        'include/freetype/ftchapters.h',
        'include/freetype/ftcid.h',
        'include/freetype/fterrdef.h',
        'include/freetype/fterrors.h',
        'include/freetype/ftfntfmt.h',
        'include/freetype/ftgasp.h',
        'include/freetype/ftglyph.h',
        'include/freetype/ftgxval.h',
        'include/freetype/ftgzip.h',
        'include/freetype/ftimage.h',
        'include/freetype/ftincrem.h',
        'include/freetype/ftlcdfil.h',
        'include/freetype/ftlist.h',
        'include/freetype/ftlzw.h',
        'include/freetype/ftmac.h',
        'include/freetype/ftmm.h',
        'include/freetype/ftmodapi.h',
        'include/freetype/ftmoderr.h',
        'include/freetype/ftotval.h',
        'include/freetype/ftoutln.h',
        'include/freetype/ftpfr.h',
        'include/freetype/ftrender.h',
        'include/freetype/ftsizes.h',
        'include/freetype/ftsnames.h',
        'include/freetype/ftstroke.h',
        'include/freetype/ftsynth.h',
        'include/freetype/ftsystem.h',
        'include/freetype/fttrigon.h',
        'include/freetype/ftttdrv.h',
        'include/freetype/fttypes.h',
        'include/freetype/ftwinfnt.h',
        'include/freetype/t1tables.h',
        'include/freetype/ttnameid.h',
        'include/freetype/tttables.h',
        'include/freetype/tttags.h',
        'include/freetype/ttunpat.h',
        # 
        # src
        # 
        # base
        'src/base/ftbase.c',
        'src/base/ftbbox.c',
        'src/base/ftbdf.c',
        'src/base/ftbitmap.c',
        'src/base/ftcid.c',
        'src/base/ftdebug.c',
        'src/base/ftfntfmt.c',
        'src/base/ftfstype.c',
        'src/base/ftgasp.c',
        'src/base/ftglyph.c',
        'src/base/ftgxval.c',
        'src/base/ftinit.c',
        'src/base/ftlcdfil.c',
        'src/base/ftmm.c',
        'src/base/ftotval.c',
        'src/base/ftpatent.c',
        'src/base/ftpfr.c',
        'src/base/ftstroke.c',
        'src/base/ftsynth.c',
        'src/base/ftsystem.c',
        'src/base/fttype1.c',
        'src/base/ftwinfnt.c',
        # bzip2
        'src/bzip2/ftbzip2.c',
        # cache
        'src/cache/ftcache.c',
        # gzip
        'src/gzip/ftgzip.c',
        # lzw
        'src/lzw/ftlzw.c',
        #
        # -------------- module --------------
        # autofit
        'src/autofit/autofit.c',
        # bdf
        'src/bdf/bdf.c',
        # cff
        'src/cff/cff.c',
        # cid
        'src/cid/type1cid.c',
        # gxvalid
        'src/gxvalid/gxvalid.c',
        # otvalid
        'src/otvalid/otvalid.c',
        # pcf
        'src/pcf/pcf.c',
        # pfr
        'src/pfr/pfr.c',
        # psaux
        'src/psaux/psaux.c',
        # pshinter
        'src/pshinter/pshinter.c',
        # psnames
        'src/psnames/psnames.c',
        # raster
        'src/raster/raster.c',
        # sfnt
        'src/sfnt/sfnt.c',
        # smooth
        'src/smooth/smooth.c',
        # truetype
        'src/truetype/truetype.c',
        # type1
        'src/type1/type1.c',
        # type42
        'src/type42/type42.c',
        # winfonts
        'src/winfonts/winfnt.c',
      ]
    }
  ]  
}