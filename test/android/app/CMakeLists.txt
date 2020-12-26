
cmake_minimum_required(VERSION 3.4.1)

include_directories(../../..)
include_directories(../../../depe/rapidjson/include)
include_directories(../../../depe/freetype2/include)
include_directories(../../../depe/tess2/Include)
include_directories(../../../depe/libwebp)
include_directories(../../../depe/libgif/lib)
include_directories(../../../depe/libjpeg)
include_directories(../../../depe/libpng)
include_directories(../../../depe/libwebp)
include_directories(../../../depe/ffmpeg)
include_directories(../../../depe/tinyxml2)
include_directories(../../../depe/node/src)
include_directories(../../../depe/node/deps/cares/include)
include_directories(../../../depe/node/deps/zlib/contrib/minizip)
include_directories(../../../depe/node/deps/openssl/openssl/include)
include_directories(../../../depe/node/deps/uv/include)
include_directories(../../../depe/node/deps/http_parser)
include_directories(../../../depe/v8-link/include)
include_directories(../../../depe/bplus/include)
include_directories(../../../out)
link_directories(src/main/jniLibs/${ANDROID_ABI})
link_directories(../../../out/jniLibs/${ANDROID_ABI})

add_definitions(
	-D__STDC_CONSTANT_MACROS
	-DFX_BUILDING_SHARED
	-DNODE_WANT_INTERNALS=1
	-DHAVE_OPENSSL=1
	-DHAVE_INSPECTOR=1
	-DCHROME_PNG_WRITE_SUPPORT
	-DPNG_USER_CONFIG
)

set(TARGET "bplus")
set(TOOLSET "target")
add_library(bplus STATIC
	../../../depe/bplus/bplus.c
	../../../depe/bplus/pages.c
	../../../depe/bplus/utils.c
	../../../depe/bplus/values.c
	../../../depe/bplus/writer.c
)
set_target_properties(bplus PROPERTIES COMPILE_DEFINITIONS 
	"_LARGEFILE_SOURCE;_FILE_OFFSET_BITS=64;_XOPEN_SOURCE=500;_DARWIN_C_SOURCE;NDEBUG")
set_target_properties(bplus PROPERTIES COMPILE_FLAGS "-std=c99 -pedantic -pthread -O3")
unset(TOOLSET)
unset(TARGET)

add_definitions(-DDEBUG)

add_library(ftr SHARED
	# ftr-util
	../../../android/android.cc
	../../../ftr/util/_android.cc
	../../../ftr/util/android-jni.cc
	../../../ftr/util/android-log.cc
	../../../ftr/util/object.cc
	../../../ftr/util/string.cc
	../../../ftr/util/string-builder.cc
	../../../ftr/util/array.cc
	../../../ftr/util/codec.cc
	../../../ftr/util/error.cc
	../../../ftr/util/http.cc
	../../../ftr/util/http-uri.cc
	../../../ftr/util/http-helper.cc
	../../../ftr/util/fs.cc
	../../../ftr/util/fs-file.cc
	../../../ftr/util/fs-sync.cc
	../../../ftr/util/fs-async.cc
	../../../ftr/util/fs-reader.cc
	../../../ftr/util/buffer.cc
	../../../ftr/util/json.cc
	../../../ftr/util/map.cc
	../../../ftr/util/util.cc
	../../../ftr/util/zlib.cc
	../../../ftr/util/loop.cc
	../../../ftr/util/loop-private.cc
	../../../ftr/util/net.cc
	../../../ftr/util/cb.cc
	../../../ftr/util/date.cc
	../../../ftr/util/http-cookie.cc
	../../../ftr/util/localstorage.cc
	# ftr
	../../../out/native-glsl.cc
	../../../out/native-font.cc
	../../../ftr/action.cc
	../../../ftr/app.cc
	../../../ftr/div.cc
	../../../ftr/indep.cc
	../../../ftr/box-shadow.cc
	../../../ftr/limit.cc
	../../../ftr/limit-indep.cc
	../../../ftr/image.cc
	../../../ftr/bezier.cc
	../../../ftr/event.cc
	../../../ftr/display-port.cc
	../../../ftr/font/font.cc
	../../../ftr/image/codec.cc
	../../../ftr/image/codec-tga.cc
	../../../ftr/image/codec-pvrtc.cc
	../../../ftr/pre-render.cc
	../../../ftr/mathe.cc
	../../../ftr/label.cc
	../../../ftr/layout.cc
	../../../ftr/box.cc
	../../../ftr/text-rows.cc
	../../../ftr/view.cc
	../../../ftr/draw.cc
	../../../ftr/gl/gl.cc
	../../../ftr/gl/gl-draw.cc
	../../../ftr/gl/gl-texture.cc
	../../../ftr/gl/gl-font.cc
	../../../ftr/root.cc
	../../../ftr/sprite.cc
	../../../ftr/scroll.cc
	../../../ftr/span.cc
	../../../ftr/hybrid.cc
	../../../ftr/text-font.cc
	../../../ftr/text-node.cc
	../../../ftr/texture.cc
	../../../ftr/value.cc
	../../../ftr/panel.cc
	../../../ftr/button.cc
	../../../ftr/keyboard.cc
	../../../ftr/css.cc
	../../../ftr/property.cc
	../../../ftr/text.cc
	../../../ftr/input.cc
	../../../ftr/textarea.cc
	../../../ftr/background.cc
	../../../ftr/render-looper.cc
	../../../ftr/sys.cc
	../../../ftr/platforms/linux-gl.cc
	../../../ftr/platforms/android-app.cc
	../../../ftr/platforms/android-keyboard.cc
	../../../ftr/platforms/android-sys.cc
	../../../ftr/image/codec-gif.cc
  ../../../ftr/image/codec-jpeg.cc
  ../../../ftr/image/codec-png.cc
  ../../../ftr/image/codec-webp.cc
  ../../../ftr/media.cc
)

add_library(ftr-media SHARED
	../../../ftr/audio-player.cc
	../../../ftr/video.cc
	../../../ftr/media-codec.cc
	../../../ftr/media-codec-1.cc
	../../../ftr/media-codec-software.cc
	../../../ftr/media-init.cc
	../../../ftr/platforms/android-media-codec.cc
	../../../ftr/platforms/android-pcm-player.cc
	../../../ftr/platforms/android-pcm-audio-track.cc
)

add_library(ftr-js SHARED
	../../../out/native-inl-js.cc
	../../../out/native-lib-js.cc
	../../../out/native-ext-js.cc
	../../../ftr-js/js-cls.cc
	../../../ftr-js/js.cc
	../../../ftr-js/ftr.cc
	../../../ftr-js/str.cc
	../../../ftr-js/v8.cc
	../../../ftr-js/wrap.cc
	../../../ftr-js/node.cc
	# binding ftr-util
	../../../ftr-js/binding/cb.cc
	../../../ftr-js/binding/fs.cc
	../../../ftr-js/binding/fs-reader.cc
	../../../ftr-js/binding/fs-path.cc
	../../../ftr-js/binding/http.cc
	../../../ftr-js/binding/util.cc
	../../../ftr-js/binding/storage.cc
	../../../ftr-js/binding/json.cc
	../../../ftr-js/binding/event.cc
	../../../ftr-js/binding/sys.cc
	../../../ftr-js/binding/event.cc
	../../../ftr-js/binding/timer.cc
	../../../ftr-js/binding/console.cc
	../../../ftr-js/binding/buffer.cc
	# binding ftr-gui
	../../../ftr-js/binding/value.cc
	../../../ftr-js/binding/binding.cc
	../../../ftr-js/binding/action.cc
	../../../ftr-js/binding/action-frame.cc
	../../../ftr-js/binding/app.cc
	../../../ftr-js/binding/audio-player.cc
	../../../ftr-js/binding/video.cc
	../../../ftr-js/binding/media.cc
	../../../ftr-js/binding/div.cc
	../../../ftr-js/binding/display-port.cc
	../../../ftr-js/binding/indep.cc
	../../../ftr-js/binding/image.cc
	../../../ftr-js/binding/layout.cc
	../../../ftr-js/binding/box.cc
	../../../ftr-js/binding/view.cc
	../../../ftr-js/binding/root.cc
	../../../ftr-js/binding/span.cc
	../../../ftr-js/binding/sprite.cc
	../../../ftr-js/binding/hybrid.cc
	../../../ftr-js/binding/text-font.cc
	../../../ftr-js/binding/text-node.cc
	../../../ftr-js/binding/label.cc
	../../../ftr-js/binding/limit.cc
	../../../ftr-js/binding/panel.cc
	../../../ftr-js/binding/button.cc
	../../../ftr-js/binding/scroll.cc
	../../../ftr-js/binding/css.cc
	../../../ftr-js/binding/font.cc
	../../../ftr-js/binding/text.cc
	../../../ftr-js/binding/input.cc
	../../../ftr-js/binding/background.cc
)

add_library(ftr-trial SHARED
	../../../trial/fs-search.cc
	../../../trial/jsx.cc
)

add_library(ftr-test SHARED
	../../../test/test.cc
	../../../test/test-ftr.cc
	../../../test/test-fs.cc
	../../../test/test-fs2.cc
	../../../test/test-gui.cc
	../../../test/test-freetype.cc
	../../../test/test-json.cc
	../../../test/test-string.cc
	../../../test/test-list.cc
	../../../test/test-map.cc
	../../../test/test-event.cc
	../../../test/test-zlib.cc
	../../../test/test-http.cc
	../../../test/test-http2.cc
	../../../test/test-http3.cc
	../../../test/test-https.cc
	../../../test/test-thread.cc
	../../../test/test-ffmpeg.cc
	../../../test/test-number.cc
	../../../test/test-uv.cc
	../../../test/test-net.cc
	../../../test/test-fs-async.cc
	../../../test/test-ssl.cc
	../../../test/test-net-ssl.cc
	../../../test/test-http-cookie.cc
	../../../test/test-localstorage.cc
	../../../test/test-buffer.cc
	../../../test/test-demo.cc
	../../../test/test-jsc.cc
	../../../test/test-v8.cc
	../../../test/test-loop.cc
	../../../test/test-sys.cc
	../../../test/test-mutex.cc
	../../../test/test-ios-run-loop.cc
	../../../test/test-benchmark.cc
	../../../test/test-sizeof.cc
	../../../test/test-util.cc
	../../../test/test-alsa-ff.cc
	../../../test/test-linux-input.cc
	../../../test/test-linux-input-2.cc
)

target_link_libraries(ftr       atomic z android log OpenSLES GLESv3 EGL mediandk ftr-depes-test)
target_link_libraries(ftr-media ftr OpenSLES GLESv3 EGL mediandk ftr-depes-test)
target_link_libraries(ftr-js    ftr ftr-depes-test)
target_link_libraries(ftr-trial ftr ftr-depes-test)
target_link_libraries(ftr-test  ftr ftr-js ftr-trial ftr-depes-test)
