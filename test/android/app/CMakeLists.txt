
cmake_minimum_required(VERSION 3.4.1)

include_directories(../../..)
include_directories(../../../depe/rapidjson/include)
include_directories(../../../depe/freetype2/include)
include_directories(../../../depe/tess2/Include)
include_directories(../../../depe/libwebp)
include_directories(../../../depe/libgif/lib)
include_directories(../../../depe/libjpeg-turbo)
include_directories(../../../depe/libpng)
include_directories(../../../depe/libwebp)
include_directories(../../../depe/FFmpeg)
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
	-DXX_BUILDING_SHARED
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

add_library(langou SHARED
	# langou-utils
	../../../android/android.cc
	../../../langou/utils/_android.cc
	../../../langou/utils/android-jni.cc
	../../../langou/utils/android-log.cc
	../../../langou/utils/object.cc
	../../../langou/utils/string.cc
	../../../langou/utils/string-builder.cc
	../../../langou/utils/array.cc
	../../../langou/utils/codec.cc
	../../../langou/utils/error.cc
	../../../langou/utils/http.cc
	../../../langou/utils/http-uri.cc
	../../../langou/utils/http-helper.cc
	../../../langou/utils/fs.cc
	../../../langou/utils/fs-file.cc
	../../../langou/utils/fs-sync.cc
	../../../langou/utils/fs-async.cc
	../../../langou/utils/fs-search.cc
	../../../langou/utils/fs-reader.cc
	../../../langou/utils/buffer.cc
	../../../langou/utils/json.cc
	../../../langou/utils/map.cc
	../../../langou/utils/util.cc
	../../../langou/utils/zlib.cc
	../../../langou/utils/loop.cc
	../../../langou/utils/loop-private.cc
	../../../langou/utils/jsx.cc
	../../../langou/utils/net.cc
	../../../langou/utils/cb.cc
	../../../langou/utils/date.cc
	../../../langou/utils/http-cookie.cc
	../../../langou/utils/localstorage.cc
	# langou
	../../../out/native-glsl.cc
	../../../out/native-font.cc
	../../../langou/action.cc
	../../../langou/app.cc
	../../../langou/div.cc
	../../../langou/indep.cc
	../../../langou/box-shadow.cc
	../../../langou/limit.cc
	../../../langou/limit-indep.cc
	../../../langou/image.cc
	../../../langou/bezier.cc
	../../../langou/event.cc
	../../../langou/display-port.cc
	../../../langou/font/font.cc
	../../../langou/image/codec.cc
	../../../langou/image/codec-tga.cc
	../../../langou/image/codec-pvrtc.cc
	../../../langou/pre-render.cc
	../../../langou/mathe.cc
	../../../langou/label.cc
	../../../langou/layout.cc
	../../../langou/box.cc
	../../../langou/text-rows.cc
	../../../langou/view.cc
	../../../langou/draw.cc
	../../../langou/gl/gl.cc
	../../../langou/gl/gl-draw.cc
	../../../langou/gl/gl-texture.cc
	../../../langou/gl/gl-font.cc
	../../../langou/root.cc
	../../../langou/sprite.cc
	../../../langou/scroll.cc
	../../../langou/span.cc
	../../../langou/hybrid.cc
	../../../langou/text-font.cc
	../../../langou/text-node.cc
	../../../langou/texture.cc
	../../../langou/value.cc
	../../../langou/panel.cc
	../../../langou/button.cc
	../../../langou/keyboard.cc
	../../../langou/css.cc
	../../../langou/property.cc
	../../../langou/text.cc
	../../../langou/input.cc
	../../../langou/textarea.cc
	../../../langou/background.cc
	../../../langou/render-looper.cc
	../../../langou/sys.cc
	../../../langou/platforms/linux-gl.cc
	../../../langou/platforms/android-app.cc
	../../../langou/platforms/android-keyboard.cc
	../../../langou/platforms/android-sys.cc
	../../../langou/image/codec-gif.cc
  ../../../langou/image/codec-jpeg.cc
  ../../../langou/image/codec-png.cc
  ../../../langou/image/codec-webp.cc
  ../../../langou/media.cc
)

add_library(langou-media SHARED
	../../../langou/audio-player.cc
	../../../langou/video.cc
	../../../langou/media-codec.cc
	../../../langou/media-codec-1.cc
	../../../langou/media-codec-software.cc
	../../../langou/media-init.cc
	../../../langou/platforms/android-media-codec.cc
	../../../langou/platforms/android-pcm-player.cc
	../../../langou/platforms/android-pcm-audio-track.cc
)

add_library(langou-js SHARED
	../../../out/native-inl-js.cc
	../../../out/native-lib-js.cc
	../../../out/native-ext-js.cc
	../../../langou/js/js-cls.cc
	../../../langou/js/js.cc
	../../../langou/js/langou.cc
	../../../langou/js/str.cc
	../../../langou/js/v8.cc
	../../../langou/js/wrap.cc
	../../../langou/js/node.cc
	# binding langou-utils
	../../../langou/js/binding/cb.cc
	../../../langou/js/binding/fs.cc
	../../../langou/js/binding/fs-reader.cc
	../../../langou/js/binding/fs-path.cc
	../../../langou/js/binding/http.cc
	../../../langou/js/binding/util.cc
	../../../langou/js/binding/storage.cc
	../../../langou/js/binding/json.cc
	../../../langou/js/binding/event.cc
	../../../langou/js/binding/sys.cc
	../../../langou/js/binding/event.cc
	../../../langou/js/binding/timer.cc
	../../../langou/js/binding/console.cc
	../../../langou/js/binding/buffer.cc
	# binding langou-gui
	../../../langou/js/binding/value.cc
	../../../langou/js/binding/langou-binding.cc
	../../../langou/js/binding/action.cc
	../../../langou/js/binding/action-frame.cc
	../../../langou/js/binding/app.cc
	../../../langou/js/binding/audio-player.cc
	../../../langou/js/binding/video.cc
	../../../langou/js/binding/media.cc
	../../../langou/js/binding/div.cc
	../../../langou/js/binding/display-port.cc
	../../../langou/js/binding/indep.cc
	../../../langou/js/binding/image.cc
	../../../langou/js/binding/layout.cc
	../../../langou/js/binding/box.cc
	../../../langou/js/binding/view.cc
	../../../langou/js/binding/root.cc
	../../../langou/js/binding/span.cc
	../../../langou/js/binding/sprite.cc
	../../../langou/js/binding/hybrid.cc
	../../../langou/js/binding/text-font.cc
	../../../langou/js/binding/text-node.cc
	../../../langou/js/binding/label.cc
	../../../langou/js/binding/limit.cc
	../../../langou/js/binding/panel.cc
	../../../langou/js/binding/button.cc
	../../../langou/js/binding/scroll.cc
	../../../langou/js/binding/css.cc
	../../../langou/js/binding/font.cc
	../../../langou/js/binding/text.cc
	../../../langou/js/binding/input.cc
	../../../langou/js/binding/background.cc
)

add_library(langou-test SHARED
	../../../test/test.cc
	../../../test/test-langou.cc
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

target_link_libraries(langou       atomic z android log OpenSLES GLESv3 EGL mediandk langou-depes-test)
target_link_libraries(langou-media langou OpenSLES GLESv3 EGL mediandk langou-depes-test)
target_link_libraries(langou-js    langou langou-depes-test)
target_link_libraries(langou-test  langou langou-js langou-depes-test)
