
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

add_library(quark SHARED
	# quark-util
	../../../android/android.cc
	../../../quark/util/_android.cc
	../../../quark/util/android-jni.cc
	../../../quark/util/android-log.cc
	../../../quark/util/object.cc
	../../../quark/util/string.cc
	../../../quark/util/string-builder.cc
	../../../quark/util/array.cc
	../../../quark/util/codec.cc
	../../../quark/util/error.cc
	../../../quark/util/http.cc
	../../../quark/util/http-uri.cc
	../../../quark/util/http-helper.cc
	../../../quark/util/fs.cc
	../../../quark/util/fs-file.cc
	../../../quark/util/fs-sync.cc
	../../../quark/util/fs-async.cc
	../../../quark/util/fs-reader.cc
	../../../quark/util/buffer.cc
	../../../quark/util/json.cc
	../../../quark/util/map.cc
	../../../quark/util/util.cc
	../../../quark/util/zlib.cc
	../../../quark/util/loop.cc
	../../../quark/util/loop-private.cc
	../../../quark/util/net.cc
	../../../quark/util/cb.cc
	../../../quark/util/date.cc
	../../../quark/util/http-cookie.cc
	../../../quark/util/localstorage.cc
	# quark
	../../../out/native-glsl.cc
	../../../out/native-font.cc
	../../../quark/action.cc
	../../../quark/app.cc
	../../../quark/div.cc
	../../../quark/indep.cc
	../../../quark/box-shadow.cc
	../../../quark/limit.cc
	../../../quark/limit-indep.cc
	../../../quark/image.cc
	../../../quark/bezier.cc
	../../../quark/event.cc
	../../../quark/display-port.cc
	../../../quark/font/font.cc
	../../../quark/image/codec.cc
	../../../quark/image/codec-tga.cc
	../../../quark/image/codec-pvrtc.cc
	../../../quark/pre-render.cc
	../../../quark/mathe.cc
	../../../quark/label.cc
	../../../quark/layout.cc
	../../../quark/box.cc
	../../../quark/text-rows.cc
	../../../quark/view.cc
	../../../quark/draw.cc
	../../../quark/gl/gl.cc
	../../../quark/gl/gl-draw.cc
	../../../quark/gl/gl-texture.cc
	../../../quark/gl/gl-font.cc
	../../../quark/root.cc
	../../../quark/sprite.cc
	../../../quark/scroll.cc
	../../../quark/span.cc
	../../../quark/hybrid.cc
	../../../quark/text-font.cc
	../../../quark/text-node.cc
	../../../quark/texture.cc
	../../../quark/value.cc
	../../../quark/panel.cc
	../../../quark/button.cc
	../../../quark/keyboard.cc
	../../../quark/css.cc
	../../../quark/property.cc
	../../../quark/text.cc
	../../../quark/input.cc
	../../../quark/textarea.cc
	../../../quark/background.cc
	../../../quark/render-looper.cc
	../../../quark/sys.cc
	../../../quark/platforms/linux-gl.cc
	../../../quark/platforms/android-app.cc
	../../../quark/platforms/android-keyboard.cc
	../../../quark/platforms/android-sys.cc
	../../../quark/image/codec-gif.cc
  ../../../quark/image/codec-jpeg.cc
  ../../../quark/image/codec-png.cc
  ../../../quark/image/codec-webp.cc
  ../../../quark/media.cc
)

add_library(quark-media SHARED
	../../../quark/audio-player.cc
	../../../quark/video.cc
	../../../quark/media-codec.cc
	../../../quark/media-codec-1.cc
	../../../quark/media-codec-software.cc
	../../../quark/media-init.cc
	../../../quark/platforms/android-media-codec.cc
	../../../quark/platforms/android-pcm-player.cc
	../../../quark/platforms/android-pcm-audio-track.cc
)

add_library(quark-js SHARED
	../../../out/native-inl-js.cc
	../../../out/native-lib-js.cc
	../../../out/native-ext-js.cc
	../../../quark-js/js-cls.cc
	../../../quark-js/js.cc
	../../../quark-js/quarks.cc
	../../../quark-js/str.cc
	../../../quark-js/v8.cc
	../../../quark-js/wrap.cc
	../../../quark-js/node.cc
	# binding quark-util
	../../../quark-js/binding/cb.cc
	../../../quark-js/binding/fs.cc
	../../../quark-js/binding/fs-reader.cc
	../../../quark-js/binding/fs-path.cc
	../../../quark-js/binding/http.cc
	../../../quark-js/binding/util.cc
	../../../quark-js/binding/storage.cc
	../../../quark-js/binding/json.cc
	../../../quark-js/binding/event.cc
	../../../quark-js/binding/sys.cc
	../../../quark-js/binding/event.cc
	../../../quark-js/binding/timer.cc
	../../../quark-js/binding/console.cc
	../../../quark-js/binding/buffer.cc
	# binding quark-gui
	../../../quark-js/binding/value.cc
	../../../quark-js/binding/binding.cc
	../../../quark-js/binding/action.cc
	../../../quark-js/binding/action-frame.cc
	../../../quark-js/binding/app.cc
	../../../quark-js/binding/audio-player.cc
	../../../quark-js/binding/video.cc
	../../../quark-js/binding/media.cc
	../../../quark-js/binding/div.cc
	../../../quark-js/binding/display-port.cc
	../../../quark-js/binding/indep.cc
	../../../quark-js/binding/image.cc
	../../../quark-js/binding/layout.cc
	../../../quark-js/binding/box.cc
	../../../quark-js/binding/view.cc
	../../../quark-js/binding/root.cc
	../../../quark-js/binding/span.cc
	../../../quark-js/binding/sprite.cc
	../../../quark-js/binding/hybrid.cc
	../../../quark-js/binding/text-font.cc
	../../../quark-js/binding/text-node.cc
	../../../quark-js/binding/label.cc
	../../../quark-js/binding/limit.cc
	../../../quark-js/binding/panel.cc
	../../../quark-js/binding/button.cc
	../../../quark-js/binding/scroll.cc
	../../../quark-js/binding/css.cc
	../../../quark-js/binding/font.cc
	../../../quark-js/binding/text.cc
	../../../quark-js/binding/input.cc
	../../../quark-js/binding/background.cc
)

add_library(quark-trial SHARED
	../../../trial/fs-search.cc
	../../../trial/jsx.cc
)

add_library(quark-test SHARED
	../../../test/test.cc
	#../../../test/test-quarks.cc
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

target_link_libraries(quark       atomic z android log OpenSLES GLESv3 EGL mediandk quark-depes-test)
target_link_libraries(quark-media quark OpenSLES GLESv3 EGL mediandk quark-depes-test)
target_link_libraries(quark-js    quark quark-depes-test)
target_link_libraries(quark-trial quark quark-depes-test)
target_link_libraries(quark-test  quark quark-js quark-trial quark-depes-test)
