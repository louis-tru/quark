
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
	-DNX_BUILDING_SHARED
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

add_library(ngui SHARED
	# nxkit
	../../../android/android.cc
	../../../nxkit/_android.cc
	../../../nxkit/android-jni.cc
	../../../nxkit/android-log.cc
	../../../nxkit/object.cc
	../../../nxkit/string.cc
	../../../nxkit/string-builder.cc
	../../../nxkit/array.cc
	../../../nxkit/codec.cc
	../../../nxkit/error.cc
	../../../nxkit/http.cc
	../../../nxkit/http-uri.cc
	../../../nxkit/http-helper.cc
	../../../nxkit/fs.cc
	../../../nxkit/fs-file.cc
	../../../nxkit/fs-sync.cc
	../../../nxkit/fs-async.cc
	../../../nxkit/fs-search.cc
	../../../nxkit/fs-reader.cc
	../../../nxkit/buffer.cc
	../../../nxkit/json.cc
	../../../nxkit/map.cc
	../../../nxkit/util.cc
	../../../nxkit/zlib.cc
	../../../nxkit/loop.cc
	../../../nxkit/loop-private.cc
	../../../nxkit/jsx.cc
	../../../nxkit/net.cc
	../../../nxkit/cb.cc
	../../../nxkit/date.cc
	../../../nxkit/http-cookie.cc
	../../../nxkit/localstorage.cc
	# ngui
	../../../out/native-glsl.cc
	../../../out/native-font.cc
	../../../ngui/action.cc
	../../../ngui/app.cc
	../../../ngui/div.cc
	../../../ngui/indep.cc
	../../../ngui/box-shadow.cc
	../../../ngui/limit.cc
	../../../ngui/limit-indep.cc
	../../../ngui/image.cc
	../../../ngui/bezier.cc
	../../../ngui/event.cc
	../../../ngui/display-port.cc
	../../../ngui/font/font.cc
	../../../ngui/image/codec.cc
	../../../ngui/image/codec-tga.cc
	../../../ngui/image/codec-pvrtc.cc
	../../../ngui/pre-render.cc
	../../../ngui/mathe.cc
	../../../ngui/label.cc
	../../../ngui/layout.cc
	../../../ngui/box.cc
	../../../ngui/text-rows.cc
	../../../ngui/view.cc
	../../../ngui/draw.cc
	../../../ngui/gl/gl.cc
	../../../ngui/gl/gl-draw.cc
	../../../ngui/gl/gl-texture.cc
	../../../ngui/gl/gl-font.cc
	../../../ngui/root.cc
	../../../ngui/sprite.cc
	../../../ngui/scroll.cc
	../../../ngui/span.cc
	../../../ngui/hybrid.cc
	../../../ngui/text-font.cc
	../../../ngui/text-node.cc
	../../../ngui/texture.cc
	../../../ngui/value.cc
	../../../ngui/panel.cc
	../../../ngui/button.cc
	../../../ngui/keyboard.cc
	../../../ngui/css.cc
	../../../ngui/property.cc
	../../../ngui/text.cc
	../../../ngui/input.cc
	../../../ngui/textarea.cc
	../../../ngui/background.cc
	../../../ngui/render-looper.cc
	../../../ngui/sys.cc
	../../../ngui/platforms/linux-gl.cc
	../../../ngui/platforms/android-app.cc
	../../../ngui/platforms/android-keyboard.cc
	../../../ngui/platforms/android-sys.cc
	../../../ngui/image/codec-gif.cc
  ../../../ngui/image/codec-jpeg.cc
  ../../../ngui/image/codec-png.cc
  ../../../ngui/image/codec-webp.cc
  ../../../ngui/media.cc
)

add_library(nxmedia SHARED
	../../../ngui/audio-player.cc
	../../../ngui/video.cc
	../../../ngui/media-codec.cc
	../../../ngui/media-codec-1.cc
	../../../ngui/media-codec-software.cc
	../../../ngui/media-init.cc
	../../../ngui/platforms/android-media-codec.cc
	../../../ngui/platforms/android-pcm-player.cc
	../../../ngui/platforms/android-pcm-audio-track.cc
)

add_library(nxjs SHARED
	../../../out/native-inl-js.cc
	../../../out/native-lib-js.cc
	../../../out/native-ext-js.cc
	../../../ngui/js/js-cls.cc
	../../../ngui/js/js.cc
	../../../ngui/js/ngui.cc
	../../../ngui/js/str.cc
	../../../ngui/js/v8.cc
	../../../ngui/js/wrap.cc
	../../../ngui/js/node.cc
	# binding nxkit
	../../../ngui/js/binding/cb.cc
	../../../ngui/js/binding/fs.cc
	../../../ngui/js/binding/fs-reader.cc
	../../../ngui/js/binding/fs-path.cc
	../../../ngui/js/binding/http.cc
	../../../ngui/js/binding/util.cc
	../../../ngui/js/binding/storage.cc
	../../../ngui/js/binding/json.cc
	../../../ngui/js/binding/event.cc
	../../../ngui/js/binding/sys.cc
	../../../ngui/js/binding/event.cc
	../../../ngui/js/binding/timer.cc
	../../../ngui/js/binding/console.cc
	../../../ngui/js/binding/buffer.cc
	# binding ngui-gui
	../../../ngui/js/binding/value.cc
	../../../ngui/js/binding/ngui-binding.cc
	../../../ngui/js/binding/action.cc
	../../../ngui/js/binding/action-frame.cc
	../../../ngui/js/binding/app.cc
	../../../ngui/js/binding/audio-player.cc
	../../../ngui/js/binding/video.cc
	../../../ngui/js/binding/media.cc
	../../../ngui/js/binding/div.cc
	../../../ngui/js/binding/display-port.cc
	../../../ngui/js/binding/indep.cc
	../../../ngui/js/binding/image.cc
	../../../ngui/js/binding/layout.cc
	../../../ngui/js/binding/box.cc
	../../../ngui/js/binding/view.cc
	../../../ngui/js/binding/root.cc
	../../../ngui/js/binding/span.cc
	../../../ngui/js/binding/sprite.cc
	../../../ngui/js/binding/hybrid.cc
	../../../ngui/js/binding/text-font.cc
	../../../ngui/js/binding/text-node.cc
	../../../ngui/js/binding/label.cc
	../../../ngui/js/binding/limit.cc
	../../../ngui/js/binding/panel.cc
	../../../ngui/js/binding/button.cc
	../../../ngui/js/binding/scroll.cc
	../../../ngui/js/binding/css.cc
	../../../ngui/js/binding/font.cc
	../../../ngui/js/binding/text.cc
	../../../ngui/js/binding/input.cc
	../../../ngui/js/binding/background.cc
)

add_library(ngui-test SHARED
	../../../test/test.cc
	../../../test/test-ngui.cc
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

target_link_libraries(ngui       atomic z android log OpenSLES GLESv3 EGL mediandk ngui-depes-test)
target_link_libraries(nxmedia ngui OpenSLES GLESv3 EGL mediandk ngui-depes-test)
target_link_libraries(nxjs    ngui ngui-depes-test)
target_link_libraries(ngui-test  ngui nxjs ngui-depes-test)
