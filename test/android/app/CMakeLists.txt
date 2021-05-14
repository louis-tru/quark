
cmake_minimum_required(VERSION 3.4.1)

include_directories(../../..)
include_directories(../../../deps/rapidjson/include)
include_directories(../../../deps/freetype2/include)
include_directories(../../../deps/tess2/Include)
include_directories(../../../deps/libwebp)
include_directories(../../../deps/libgif/lib)
include_directories(../../../deps/libjpeg)
include_directories(../../../deps/libpng)
include_directories(../../../deps/libwebp)
include_directories(../../../deps/ffmpeg)
include_directories(../../../deps/tinyxml2)
include_directories(../../../deps/node/src)
include_directories(../../../deps/node/deps/cares/include)
include_directories(../../../deps/node/deps/zlib/contrib/minizip)
include_directories(../../../deps/node/deps/openssl/openssl/include)
include_directories(../../../deps/node/deps/uv/include)
include_directories(../../../deps/node/deps/http_parser)
include_directories(../../../deps/v8-link/include)
include_directories(../../../deps/bplus/include)
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
	../../../deps/bplus/bplus.c
	../../../deps/bplus/pages.c
	../../../deps/bplus/utils.c
	../../../deps/bplus/values.c
	../../../deps/bplus/writer.c
)
set_target_properties(bplus PROPERTIES COMPILE_DEFINITIONS 
	"_LARGEFILE_SOURCE;_FILE_OFFSET_BITS=64;_XOPEN_SOURCE=500;_DARWIN_C_SOURCE;NDEBUG")
set_target_properties(bplus PROPERTIES COMPILE_FLAGS "-std=c99 -pedantic -pthread -O3")
unset(TOOLSET)
unset(TARGET)

add_definitions(-DDEBUG)

add_library(flare SHARED
	# flare-util
	../../../android/android.cc
	../../../flare/util/_android.cc
	../../../flare/util/android-jni.cc
	../../../flare/util/android-log.cc
	../../../flare/util/object.cc
	../../../flare/util/string.cc
	../../../flare/util/string-builder.cc
	../../../flare/util/array.cc
	../../../flare/util/codec.cc
	../../../flare/util/error.cc
	../../../flare/util/http.cc
	../../../flare/util/http-uri.cc
	../../../flare/util/http-helper.cc
	../../../flare/util/fs.cc
	../../../flare/util/fs-file.cc
	../../../flare/util/fs-sync.cc
	../../../flare/util/fs-async.cc
	../../../flare/util/fs-reader.cc
	../../../flare/util/buffer.cc
	../../../flare/util/json.cc
	../../../flare/util/map.cc
	../../../flare/util/util.cc
	../../../flare/util/zlib.cc
	../../../flare/util/loop.cc
	../../../flare/util/loop-private.cc
	../../../flare/util/net.cc
	../../../flare/util/cb.cc
	../../../flare/util/date.cc
	../../../flare/util/http-cookie.cc
	../../../flare/util/localstorage.cc
	# flare
	../../../out/native-glsl.cc
	../../../out/native-font.cc
	../../../flare/action.cc
	../../../flare/app.cc
	../../../flare/div.cc
	../../../flare/indep.cc
	../../../flare/box-shadow.cc
	../../../flare/limit.cc
	../../../flare/limit-indep.cc
	../../../flare/image.cc
	../../../flare/bezier.cc
	../../../flare/event.cc
	../../../flare/display-port.cc
	../../../flare/font/font.cc
	../../../flare/image/codec.cc
	../../../flare/image/codec-tga.cc
	../../../flare/image/codec-pvrtc.cc
	../../../flare/pre-render.cc
	../../../flare/mathe.cc
	../../../flare/label.cc
	../../../flare/layout.cc
	../../../flare/box.cc
	../../../flare/text-rows.cc
	../../../flare/view.cc
	../../../flare/draw.cc
	../../../flare/gl/gl.cc
	../../../flare/gl/gl-draw.cc
	../../../flare/gl/gl-texture.cc
	../../../flare/gl/gl-font.cc
	../../../flare/root.cc
	../../../flare/sprite.cc
	../../../flare/scroll.cc
	../../../flare/span.cc
	../../../flare/hybrid.cc
	../../../flare/text-font.cc
	../../../flare/text-node.cc
	../../../flare/texture.cc
	../../../flare/value.cc
	../../../flare/panel.cc
	../../../flare/button.cc
	../../../flare/keyboard.cc
	../../../flare/css.cc
	../../../flare/property.cc
	../../../flare/text.cc
	../../../flare/input.cc
	../../../flare/textarea.cc
	../../../flare/background.cc
	../../../flare/render-looper.cc
	../../../flare/sys.cc
	../../../flare/platforms/linux-gl.cc
	../../../flare/platforms/android-app.cc
	../../../flare/platforms/android-keyboard.cc
	../../../flare/platforms/android-sys.cc
	../../../flare/image/codec-gif.cc
  ../../../flare/image/codec-jpeg.cc
  ../../../flare/image/codec-png.cc
  ../../../flare/image/codec-webp.cc
  ../../../flare/media.cc
)

add_library(flare-media SHARED
	../../../flare/audio-player.cc
	../../../flare/video.cc
	../../../flare/media-codec.cc
	../../../flare/media-codec-1.cc
	../../../flare/media-codec-software.cc
	../../../flare/media-init.cc
	../../../flare/platforms/android-media-codec.cc
	../../../flare/platforms/android-pcm-player.cc
	../../../flare/platforms/android-pcm-audio-track.cc
)

add_library(flare-js SHARED
	../../../out/native-inl-js.cc
	../../../out/native-lib-js.cc
	../../../out/native-ext-js.cc
	../../../flare-js/js-cls.cc
	../../../flare-js/js.cc
	../../../flare-js/flare.cc
	../../../flare-js/str.cc
	../../../flare-js/v8.cc
	../../../flare-js/wrap.cc
	../../../flare-js/node.cc
	# binding flare-util
	../../../flare-js/binding/cb.cc
	../../../flare-js/binding/fs.cc
	../../../flare-js/binding/fs-reader.cc
	../../../flare-js/binding/fs-path.cc
	../../../flare-js/binding/http.cc
	../../../flare-js/binding/util.cc
	../../../flare-js/binding/storage.cc
	../../../flare-js/binding/json.cc
	../../../flare-js/binding/event.cc
	../../../flare-js/binding/sys.cc
	../../../flare-js/binding/event.cc
	../../../flare-js/binding/timer.cc
	../../../flare-js/binding/console.cc
	../../../flare-js/binding/buffer.cc
	# binding flare-gui
	../../../flare-js/binding/value.cc
	../../../flare-js/binding/binding.cc
	../../../flare-js/binding/action.cc
	../../../flare-js/binding/action-frame.cc
	../../../flare-js/binding/app.cc
	../../../flare-js/binding/audio-player.cc
	../../../flare-js/binding/video.cc
	../../../flare-js/binding/media.cc
	../../../flare-js/binding/div.cc
	../../../flare-js/binding/display-port.cc
	../../../flare-js/binding/indep.cc
	../../../flare-js/binding/image.cc
	../../../flare-js/binding/layout.cc
	../../../flare-js/binding/box.cc
	../../../flare-js/binding/view.cc
	../../../flare-js/binding/root.cc
	../../../flare-js/binding/span.cc
	../../../flare-js/binding/sprite.cc
	../../../flare-js/binding/hybrid.cc
	../../../flare-js/binding/text-font.cc
	../../../flare-js/binding/text-node.cc
	../../../flare-js/binding/label.cc
	../../../flare-js/binding/limit.cc
	../../../flare-js/binding/panel.cc
	../../../flare-js/binding/button.cc
	../../../flare-js/binding/scroll.cc
	../../../flare-js/binding/css.cc
	../../../flare-js/binding/font.cc
	../../../flare-js/binding/text.cc
	../../../flare-js/binding/input.cc
	../../../flare-js/binding/background.cc
)

add_library(flare-trial SHARED
	../../../trial/fs-search.cc
	../../../trial/jsx.cc
)

add_library(flare-test SHARED
	../../../test/test.cc
	../../../test/test-flare.cc
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

target_link_libraries(flare       atomic z android log OpenSLES GLESv3 EGL mediandk flare-depes-test)
target_link_libraries(flare-media flare OpenSLES GLESv3 EGL mediandk flare-depes-test)
target_link_libraries(flare-js    flare flare-depes-test)
target_link_libraries(flare-trial flare flare-depes-test)
target_link_libraries(flare-test  flare flare-js flare-trial flare-depes-test)
