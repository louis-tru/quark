
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

add_library(noug SHARED
	# noug-util
	../../../android/android.cc
	../../../noug/util/_android.cc
	../../../noug/util/android-jni.cc
	../../../noug/util/android-log.cc
	../../../noug/util/object.cc
	../../../noug/util/string.cc
	../../../noug/util/string-builder.cc
	../../../noug/util/array.cc
	../../../noug/util/codec.cc
	../../../noug/util/error.cc
	../../../noug/util/http.cc
	../../../noug/util/http-uri.cc
	../../../noug/util/http-helper.cc
	../../../noug/util/fs.cc
	../../../noug/util/fs-file.cc
	../../../noug/util/fs-sync.cc
	../../../noug/util/fs-async.cc
	../../../noug/util/fs-reader.cc
	../../../noug/util/buffer.cc
	../../../noug/util/json.cc
	../../../noug/util/map.cc
	../../../noug/util/util.cc
	../../../noug/util/zlib.cc
	../../../noug/util/loop.cc
	../../../noug/util/loop-private.cc
	../../../noug/util/net.cc
	../../../noug/util/cb.cc
	../../../noug/util/date.cc
	../../../noug/util/http-cookie.cc
	../../../noug/util/localstorage.cc
	# noug
	../../../out/native-glsl.cc
	../../../out/native-font.cc
	../../../noug/action.cc
	../../../noug/app.cc
	../../../noug/div.cc
	../../../noug/indep.cc
	../../../noug/box-shadow.cc
	../../../noug/limit.cc
	../../../noug/limit-indep.cc
	../../../noug/image.cc
	../../../noug/bezier.cc
	../../../noug/event.cc
	../../../noug/display-port.cc
	../../../noug/font/font.cc
	../../../noug/image/codec.cc
	../../../noug/image/codec-tga.cc
	../../../noug/image/codec-pvrtc.cc
	../../../noug/pre-render.cc
	../../../noug/mathe.cc
	../../../noug/label.cc
	../../../noug/layout.cc
	../../../noug/box.cc
	../../../noug/text-rows.cc
	../../../noug/view.cc
	../../../noug/draw.cc
	../../../noug/gl/gl.cc
	../../../noug/gl/gl-draw.cc
	../../../noug/gl/gl-texture.cc
	../../../noug/gl/gl-font.cc
	../../../noug/root.cc
	../../../noug/sprite.cc
	../../../noug/scroll.cc
	../../../noug/span.cc
	../../../noug/hybrid.cc
	../../../noug/text-font.cc
	../../../noug/text-node.cc
	../../../noug/texture.cc
	../../../noug/value.cc
	../../../noug/panel.cc
	../../../noug/button.cc
	../../../noug/keyboard.cc
	../../../noug/css.cc
	../../../noug/property.cc
	../../../noug/text.cc
	../../../noug/input.cc
	../../../noug/textarea.cc
	../../../noug/background.cc
	../../../noug/render-looper.cc
	../../../noug/sys.cc
	../../../noug/platforms/linux-gl.cc
	../../../noug/platforms/android-app.cc
	../../../noug/platforms/android-keyboard.cc
	../../../noug/platforms/android-sys.cc
	../../../noug/image/codec-gif.cc
  ../../../noug/image/codec-jpeg.cc
  ../../../noug/image/codec-png.cc
  ../../../noug/image/codec-webp.cc
  ../../../noug/media.cc
)

add_library(noug-media SHARED
	../../../noug/audio-player.cc
	../../../noug/video.cc
	../../../noug/media-codec.cc
	../../../noug/media-codec-1.cc
	../../../noug/media-codec-software.cc
	../../../noug/media-init.cc
	../../../noug/platforms/android-media-codec.cc
	../../../noug/platforms/android-pcm-player.cc
	../../../noug/platforms/android-pcm-audio-track.cc
)

add_library(noug-js SHARED
	../../../out/native-inl-js.cc
	../../../out/native-lib-js.cc
	../../../out/native-ext-js.cc
	../../../noug-js/js-cls.cc
	../../../noug-js/js.cc
	../../../noug-js/noug.cc
	../../../noug-js/str.cc
	../../../noug-js/v8.cc
	../../../noug-js/wrap.cc
	../../../noug-js/node.cc
	# binding noug-util
	../../../noug-js/binding/cb.cc
	../../../noug-js/binding/fs.cc
	../../../noug-js/binding/fs-reader.cc
	../../../noug-js/binding/fs-path.cc
	../../../noug-js/binding/http.cc
	../../../noug-js/binding/util.cc
	../../../noug-js/binding/storage.cc
	../../../noug-js/binding/json.cc
	../../../noug-js/binding/event.cc
	../../../noug-js/binding/sys.cc
	../../../noug-js/binding/event.cc
	../../../noug-js/binding/timer.cc
	../../../noug-js/binding/console.cc
	../../../noug-js/binding/buffer.cc
	# binding noug-gui
	../../../noug-js/binding/value.cc
	../../../noug-js/binding/binding.cc
	../../../noug-js/binding/action.cc
	../../../noug-js/binding/action-frame.cc
	../../../noug-js/binding/app.cc
	../../../noug-js/binding/audio-player.cc
	../../../noug-js/binding/video.cc
	../../../noug-js/binding/media.cc
	../../../noug-js/binding/div.cc
	../../../noug-js/binding/display-port.cc
	../../../noug-js/binding/indep.cc
	../../../noug-js/binding/image.cc
	../../../noug-js/binding/layout.cc
	../../../noug-js/binding/box.cc
	../../../noug-js/binding/view.cc
	../../../noug-js/binding/root.cc
	../../../noug-js/binding/span.cc
	../../../noug-js/binding/sprite.cc
	../../../noug-js/binding/hybrid.cc
	../../../noug-js/binding/text-font.cc
	../../../noug-js/binding/text-node.cc
	../../../noug-js/binding/label.cc
	../../../noug-js/binding/limit.cc
	../../../noug-js/binding/panel.cc
	../../../noug-js/binding/button.cc
	../../../noug-js/binding/scroll.cc
	../../../noug-js/binding/css.cc
	../../../noug-js/binding/font.cc
	../../../noug-js/binding/text.cc
	../../../noug-js/binding/input.cc
	../../../noug-js/binding/background.cc
)

add_library(noug-trial SHARED
	../../../trial/fs-search.cc
	../../../trial/jsx.cc
)

add_library(noug-test SHARED
	../../../test/test.cc
	#../../../test/test-noug.cc
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

target_link_libraries(noug       atomic z android log OpenSLES GLESv3 EGL mediandk noug-depes-test)
target_link_libraries(noug-media noug OpenSLES GLESv3 EGL mediandk noug-depes-test)
target_link_libraries(noug-js    noug noug-depes-test)
target_link_libraries(noug-trial noug noug-depes-test)
target_link_libraries(noug-test  noug noug-js noug-trial noug-depes-test)
