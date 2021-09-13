ar
    Current value (from the default) = "ar"
      From //gn/BUILDCONFIG.gn:20

cc
    Current value (from the default) = "cc"
      From //gn/BUILDCONFIG.gn:21

cc_wrapper
    Current value (from the default) = ""
      From //gn/toolchain/BUILD.gn:35

clang_win
    Current value (from the default) = ""
      From //gn/BUILDCONFIG.gn:30

clang_win_version
    Current value (from the default) = ""
      From //gn/BUILDCONFIG.gn:31

current_cpu
    Current value (from the default) = ""
      (Internally set; try `gn help current_cpu`.)

current_os
    Current value (from the default) = ""
      (Internally set; try `gn help current_os`.)

cxx
    Current value (from the default) = "c++"
      From //gn/BUILDCONFIG.gn:22

dlsymutil_pool_depth
    Current value (from the default) = 6
      From //gn/toolchain/BUILD.gn:42

    dsymutil seems to kill the machine when too many processes are run in
    parallel, so we need to use a pool to limit the concurrency when passing
    large -j to Ninja (e.g. Goma build). Unfortunately this is also one of the
    slowest steps in a build, so we don't want to limit too much. Use the number
    of CPUs as a default.

extra_asmflags
    Current value (from the default) = []
      From //gn/skia/BUILD.gn:11

extra_cflags
    Current value (from the default) = []
      From //gn/skia/BUILD.gn:12

extra_cflags_c
    Current value (from the default) = []
      From //gn/skia/BUILD.gn:13

extra_cflags_cc
    Current value (from the default) = []
      From //gn/skia/BUILD.gn:14

extra_ldflags
    Current value (from the default) = []
      From //gn/skia/BUILD.gn:15

host_ar
    Current value (from the default) = "ar"
      From //gn/toolchain/BUILD.gn:6

host_cc
    Current value (from the default) = "cc"
      From //gn/toolchain/BUILD.gn:7

host_cpu
    Current value (from the default) = "x64"
      (Internally set; try `gn help host_cpu`.)

host_cxx
    Current value (from the default) = "c++"
      From //gn/toolchain/BUILD.gn:8

host_link
    Current value (from the default) = "c++"
      From //gn/toolchain/BUILD.gn:51

host_os
    Current value (from the default) = "mac"
      (Internally set; try `gn help host_os`.)

ios_min_target
    Current value (from the default) = ""
      From //gn/BUILDCONFIG.gn:33

is_component_build
    Current value = false
      From //out/ios.arm64.Release/args.gn:2
    Overridden from the default = false
      From //gn/BUILDCONFIG.gn:12

is_debug
    Current value = false
      From //out/ios.arm64.Release/args.gn:3
    Overridden from the default = false
      From //gn/BUILDCONFIG.gn:36

is_official_build
    Current value = true
      From //out/ios.arm64.Release/args.gn:1
    Overridden from the default = false
      From //gn/BUILDCONFIG.gn:11

link_pool_depth
    Current value (from the default) = -1
      From //gn/toolchain/BUILD.gn:47

    Too many linkers running at once causes issues for some builders. Allow
    such builders to limit the number of concurrent link steps.
    link_pool_depth < 0 means no pool, 0 means cpu count, > 0 sets pool size.

malloc
    Current value (from the default) = ""
      From //gn/skia/BUILD.gn:17

ndk
    Current value (from the default) = ""
      From //gn/BUILDCONFIG.gn:13

ndk_api
    Current value (from the default) = 21
      From //gn/BUILDCONFIG.gn:16

    Android 5.0, Lollipop

paragraph_bench_enabled
    Current value (from the default) = false
      From //modules/skparagraph/BUILD.gn:10

paragraph_gms_enabled
    Current value (from the default) = true
      From //modules/skparagraph/BUILD.gn:8

paragraph_tests_enabled
    Current value = false
      From //out/ios.arm64.Release/args.gn:7
    Overridden from the default = true
      From //modules/skparagraph/BUILD.gn:9

sanitize
    Current value (from the default) = ""
      From //gn/BUILDCONFIG.gn:18

skia_android_serial
    Current value (from the default) = ""
      From //gn/skia.gni:12

skia_build_fuzzers
    Current value (from the default) = false
      From //gn/skia.gni:96

skia_compile_processors
    Current value (from the default) = false
      From //gn/skia.gni:13

skia_compile_sksl_tests
    Current value (from the default) = false
      From //gn/skia.gni:105

skia_enable_android_utils
    Current value (from the default) = false
      From //gn/skia.gni:15

skia_enable_api_available_macro
    Current value (from the default) = true
      From //gn/skia.gni:14

skia_enable_direct3d_debug_layer
    Current value (from the default) = false
      From //gn/skia.gni:120

skia_enable_discrete_gpu
    Current value (from the default) = true
      From //gn/skia.gni:18

skia_enable_flutter_defines
    Current value (from the default) = false
      From //gn/skia.gni:19

skia_enable_fontmgr_FontConfigInterface
    Current value (from the default) = false
      From //gn/skia.gni:113

skia_enable_fontmgr_android
    Current value (from the default) = false
      From //gn/skia.gni:106

skia_enable_fontmgr_custom_directory
    Current value (from the default) = false
      From //gn/skia.gni:107

skia_enable_fontmgr_custom_embedded
    Current value (from the default) = false
      From //gn/skia.gni:108

skia_enable_fontmgr_custom_empty
    Current value (from the default) = false
      From //gn/skia.gni:109

skia_enable_fontmgr_empty
    Current value (from the default) = false
      From //gn/skia.gni:20

skia_enable_fontmgr_fontconfig
    Current value (from the default) = false
      From //gn/skia.gni:110

skia_enable_fontmgr_fuchsia
    Current value (from the default) = false
      From //gn/skia.gni:21

skia_enable_fontmgr_win
    Current value (from the default) = false
      From //gn/skia.gni:22

skia_enable_fontmgr_win_gdi
    Current value (from the default) = false
      From //gn/skia.gni:111

skia_enable_gpu
    Current value (from the default) = true
      From //gn/skia.gni:23

skia_enable_gpu_debug_layers
    Current value (from the default) = false
      From //gn/skia.gni:31

skia_enable_metal_debug_info
    Current value (from the default) = false
      From //gn/skia.gni:121

skia_enable_particles
    Current value (from the default) = true
      From //modules/particles/BUILD.gn:7

skia_enable_pdf
    Current value (from the default) = true
      From //gn/skia.gni:24

skia_enable_skgpu_v1
    Current value (from the default) = true
      From //gn/skia.gni:17

skia_enable_skgpu_v2
    Current value (from the default) = false
      From //gn/skia.gni:16

skia_enable_skottie
    Current value (from the default) = true
      From //gn/skia.gni:25

skia_enable_skparagraph
    Current value (from the default) = true
      From //modules/skparagraph/BUILD.gn:7

skia_enable_skrive
    Current value (from the default) = true
      From //gn/skia.gni:26

skia_enable_skshaper
    Current value (from the default) = true
      From //modules/skshaper/skshaper.gni:20

skia_enable_sksl
    Current value (from the default) = true
      From //gn/skia.gni:27

skia_enable_sktext
    Current value (from the default) = true
      From //experimental/sktext/BUILD.gn:7

skia_enable_skvm_jit_when_possible
    Current value (from the default) = false
      From //gn/skia.gni:28

skia_enable_spirv_validation
    Current value (from the default) = false
      From //gn/skia.gni:114

skia_enable_svg
    Current value (from the default) = true
      From //gn/skia.gni:29

skia_enable_tools
    Current value (from the default) = false
      From //gn/skia.gni:30

skia_enable_vulkan_debug_layers
    Current value (from the default) = false
      From //gn/skia.gni:119

skia_enable_winuwp
    Current value (from the default) = false
      From //gn/skia.gni:32

skia_fontmgr_factory
    Current value (from the default) = ":fontmgr_mac_ct_factory"
      From //gn/skia.gni:134

skia_generate_workarounds
    Current value (from the default) = false
      From //gn/skia.gni:33

skia_gl_standard
    Current value (from the default) = "gles"
      From //gn/skia.gni:83

skia_include_multiframe_procs
    Current value (from the default) = false
      From //gn/skia.gni:34

skia_ios_identity
    Current value (from the default) = ".*Google.*"
      From //gn/skia.gni:76

skia_ios_profile
    Current value (from the default) = "Google Development"
      From //gn/skia.gni:77

skia_lex
    Current value (from the default) = false
      From //gn/skia.gni:35

skia_libgifcodec_path
    Current value (from the default) = "third_party/externals/libgifcodec"
      From //gn/skia.gni:36

skia_pdf_subset_harfbuzz
    Current value (from the default) = true
      From //gn/skia.gni:101

skia_qt_path
    Current value (from the default) = ""
      From //gn/skia.gni:37

skia_skqp_global_error_tolerance
    Current value (from the default) = 0
      From //gn/skia.gni:38

skia_tools_require_resources
    Current value (from the default) = false
      From //gn/skia.gni:39

skia_update_fuchsia_sdk
    Current value (from the default) = false
      From //gn/skia.gni:40

skia_use_angle
    Current value (from the default) = false
      From //gn/skia.gni:41

skia_use_dawn
    Current value (from the default) = false
      From //gn/skia.gni:42

skia_use_direct3d
    Current value (from the default) = false
      From //gn/skia.gni:43

skia_use_dng_sdk
    Current value (from the default) = true
      From //gn/skia.gni:116

skia_use_egl
    Current value (from the default) = false
      From //gn/skia.gni:44

skia_use_expat
    Current value (from the default) = true
      From //gn/skia.gni:45

skia_use_experimental_xform
    Current value (from the default) = false
      From //gn/skia.gni:46

skia_use_ffmpeg
    Current value (from the default) = false
      From //gn/skia.gni:47

skia_use_fixed_gamma_text
    Current value (from the default) = false
      From //gn/skia.gni:48

skia_use_fontconfig
    Current value (from the default) = false
      From //gn/skia.gni:49

skia_use_fonthost_mac
    Current value (from the default) = true
      From //gn/skia.gni:50

skia_use_freetype
    Current value (from the default) = false
      From //gn/skia.gni:51

skia_use_gl
    Current value (from the default) = true
      From //gn/skia.gni:53

skia_use_harfbuzz
    Current value (from the default) = true
      From //gn/skia.gni:52

skia_use_icu
    Current value (from the default) = true
      From //gn/skia.gni:54

skia_use_libfuzzer_defaults
    Current value (from the default) = true
      From //gn/skia.gni:97

skia_use_libgifcodec
    Current value (from the default) = true
      From //gn/skia.gni:117

skia_use_libheif
    Current value (from the default) = false
      From //gn/skia.gni:55

skia_use_libjpeg_turbo_decode
    Current value = true
      From //out/ios.arm64.Release/args.gn:9
    Overridden from the default = true
      From //gn/skia.gni:56

skia_use_libjpeg_turbo_encode
    Current value = false
      From //out/ios.arm64.Release/args.gn:10
    Overridden from the default = true
      From //gn/skia.gni:57

skia_use_libpng_decode
    Current value (from the default) = true
      From //gn/skia.gni:58

skia_use_libpng_encode
    Current value (from the default) = true
      From //gn/skia.gni:59

skia_use_libwebp_decode
    Current value (from the default) = true
      From //gn/skia.gni:60

skia_use_libwebp_encode
    Current value (from the default) = true
      From //gn/skia.gni:61

skia_use_lua
    Current value (from the default) = false
      From //gn/skia.gni:62

skia_use_metal
    Current value (from the default) = false
      From //gn/skia.gni:63

skia_use_ndk_images
    Current value (from the default) = false
      From //gn/skia.gni:64

skia_use_piex
    Current value (from the default) = true
      From //gn/skia.gni:65

skia_use_runtime_icu
    Current value (from the default) = false
      From //modules/skunicode/BUILD.gn:10

skia_use_sfml
    Current value (from the default) = false
      From //gn/skia.gni:66

skia_use_sfntly
    Current value (from the default) = true
      From //gn/skia.gni:118

skia_use_system_expat
    Current value (from the default) = true
      From //third_party/expat/BUILD.gn:7

skia_use_system_harfbuzz
    Current value (from the default) = true
      From //third_party/harfbuzz/BUILD.gn:10

skia_use_system_icu
    Current value (from the default) = true
      From //third_party/icu/BUILD.gn:11

skia_use_system_libjpeg_turbo
    Current value (from the default) = true
      From //third_party/libjpeg-turbo/BUILD.gn:7

skia_use_system_libpng
    Current value (from the default) = true
      From //third_party/libpng/BUILD.gn:7

skia_use_system_libwebp
    Current value (from the default) = true
      From //third_party/libwebp/BUILD.gn:7

skia_use_system_zlib
    Current value (from the default) = true
      From //third_party/zlib/BUILD.gn:7

skia_use_vma
    Current value (from the default) = false
      From //gn/skia.gni:122

skia_use_vulkan
    Current value (from the default) = false
      From //gn/skia.gni:93

skia_use_webgl
    Current value (from the default) = false
      From //gn/skia.gni:67

skia_use_wuffs
    Current value (from the default) = false
      From //gn/skia.gni:68

skia_use_x11
    Current value (from the default) = false
      From //gn/skia.gni:69

skia_use_xps
    Current value (from the default) = true
      From //gn/skia.gni:70

skia_use_zlib
    Current value (from the default) = true
      From //gn/skia.gni:71

skia_vtune_path
    Current value (from the default) = ""
      From //gn/skia.gni:73

target_ar
    Current value (from the default) = "ar"
      From //gn/toolchain/BUILD.gn:30

target_cc
    Current value = "clang"
      From //out/ios.arm64.Release/args.gn:5
    Overridden from the default = "cc"
      From //gn/toolchain/BUILD.gn:31

target_cpu
    Current value = "arm64"
      From //out/ios.arm64.Release/args.gn:12
    Overridden from the default = ""
      (Internally set; try `gn help target_cpu`.)

target_cxx
    Current value = "clang"
      From //out/ios.arm64.Release/args.gn:4
    Overridden from the default = "c++"
      From //gn/toolchain/BUILD.gn:32

target_link
    Current value = "clang++"
      From //out/ios.arm64.Release/args.gn:6
    Overridden from the default = "clang"
      From //gn/toolchain/BUILD.gn:52

target_os
    Current value = "ios"
      From //out/ios.arm64.Release/args.gn:11
    Overridden from the default = ""
      (Internally set; try `gn help target_os`.)

text_bench_enabled
    Current value (from the default) = false
      From //experimental/sktext/BUILD.gn:10

text_gms_enabled
    Current value (from the default) = true
      From //experimental/sktext/BUILD.gn:8

text_tests_enabled
    Current value = false
      From //out/ios.arm64.Release/args.gn:8
    Overridden from the default = true
      From //experimental/sktext/BUILD.gn:9

third_party_isystem
    Current value (from the default) = true
      From //third_party/third_party.gni:7

werror
    Current value (from the default) = false
      From //gn/skia/BUILD.gn:18

win_sdk
    Current value (from the default) = "C:/Program Files (x86)/Windows Kits/10"
      From //gn/BUILDCONFIG.gn:24

win_sdk_version
    Current value (from the default) = ""
      From //gn/BUILDCONFIG.gn:25

win_toolchain_version
    Current value (from the default) = ""
      From //gn/BUILDCONFIG.gn:28

win_vc
    Current value (from the default) = ""
      From //gn/BUILDCONFIG.gn:27

xcode_sysroot
    Current value = "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS14.4.sdk"
      From //out/ios.arm64.Release/args.gn:13
    Overridden from the default = ""
      From //gn/skia/BUILD.gn:19