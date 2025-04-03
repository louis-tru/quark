
# cmake_minimum_required(VERSION 3.4.1)
cmake_minimum_required(VERSION 3.22.1)
project("quark")

include_directories(../../../out)
include_directories(../../../deps/bptree/include)
include_directories(../../../deps/ffmpeg)
include_directories(../../../deps/freetype/include)
include_directories(../../../deps/http_parser)
include_directories(../../../deps/libexpat/expat/lib)
include_directories(../../../deps/libgif/lib)
include_directories(../../../deps/libjpeg)
include_directories(../../../deps/libpng)
include_directories(../../../deps/libtess2/Include)
include_directories(../../../deps/libuv/include)
include_directories(../../../deps/libwebp/src)
include_directories(../../../deps/openssl/openssl/include)
include_directories(../../../deps/rapidjson/include)
include_directories(../../../deps/reachability)
include_directories(../../../deps/tinyxml2)
include_directories(../../../deps/v8/include)
include_directories(../../../deps/zlib/contrib/minizip)

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

# set(TARGET "bplus")
# set(TOOLSET "target")
# add_library(bplus STATIC
# 	../../../deps/bplus/bplus.c
# 	../../../deps/bplus/pages.c
# 	../../../deps/bplus/utils.c
# 	../../../deps/bplus/values.c
# 	../../../deps/bplus/writer.c
# )
# set_target_properties(bplus PROPERTIES COMPILE_DEFINITIONS 
# 	"_LARGEFILE_SOURCE;_FILE_OFFSET_BITS=64;_XOPEN_SOURCE=500;_DARWIN_C_SOURCE;NDEBUG")
# set_target_properties(bplus PROPERTIES COMPILE_FLAGS "-std=c99 -pedantic -pthread -O3")
# unset(TOOLSET)
# unset(TARGET)

add_definitions(-DDEBUG)

add_library(quark SHARED
	../../../src/util/array.cc
	../../../src/util/cb.cc
	../../../src/util/codec.cc
	../../../src/util/dict.cc
	../../../src/util/error.cc
	../../../src/util/event.cc
	../../../src/util/fs.cc
	../../../src/util/fs_async.cc
	../../../src/util/fs_path.cc
	../../../src/util/fs_reader.cc
	../../../src/util/fs_sync.cc
	../../../src/util/hash.cc
	../../../src/util/http.cc
	../../../src/util/http_cookie.cc
	../../../src/util/http_helper.cc
	../../../src/util/http_uri.cc
	../../../src/util/jni.cc
	../../../src/util/json.cc
	../../../src/util/log.cc
	../../../src/util/loop.cc
	../../../src/util/mutex.cc
	../../../src/util/net.cc
	../../../src/util/numbers.cc
	../../../src/util/object.cc
	../../../src/util/semaphore.cc
	../../../src/util/ssl.cc
	../../../src/util/storage.cc
	../../../src/util/string.cc
	../../../src/util/thread.cc
	../../../src/util/threads.cc
	../../../src/util/time.cc
	../../../src/util/util.cc
	../../../src/util/uv.cc
	../../../src/util/zlib.cc
	../../../src/media/media.cc
	../../../src/media/media_codec.cc
	../../../src/media/media_codec_software.cc
	../../../src/media/media_source.cc
	../../../src/media/player.cc
	../../../src/os/os.cc
	../../../src/platforms/android/android.cc
	../../../src/platforms/android/android_app.cc
	../../../src/platforms/android/android_keyboard.cc
	../../../src/platforms/android/android_media_codec.cc
	../../../src/platforms/android/android_os.cc
	../../../src/platforms/android/android_pcm_audio_track.cc
	../../../src/platforms/android/android_pcm_player.cc
	../../../src/platforms/android/android_screen.cc
	../../../src/platforms/android/android_window.cc
	../../../src/render/bezier.cc
	../../../src/render/canvas.cc
	../../../src/render/math.cc
	../../../src/render/paint.cc
	../../../src/render/path.cc
	../../../src/render/pathv_cache.cc
	../../../src/render/pixel.cc
	../../../src/render/render.cc
	../../../src/render/source.cc
	../../../src/render/stroke.cc
	../../../src/render/codec/codec.cc
	../../../src/render/codec/gif.cc
	../../../src/render/codec/jpeg.cc
	../../../src/render/codec/png.cc
	../../../src/render/codec/pvrtc.cc
	../../../src/render/codec/tga.cc
	../../../src/render/codec/webp.cc
	../../../src/render/font/families.cc
	../../../src/render/font/glyphs.cc
	../../../src/render/font/pool.cc
	../../../src/render/font/style.cc
	../../../src/render/font/typeface.cc
	../../../src/render/font/android/android_font.cpp
	../../../src/render/font/android/android_font_parser.cpp
	../../../src/render/font/priv/fontdata.cc
	../../../src/render/font/priv/styleset.cc
	../../../src/render/font/freetype/ft_common.cc
	../../../src/render/font/freetype/ft_glyph_cache.cc
	../../../src/render/font/freetype/ft_typeface.cc
	../../../src/render/linux/linux_render.cc
	../../../src/render/linux/linux_vulkan.cc
	../../../src/render/gl/gl_canvas.cc
	../../../src/render/gl/gl_cmd.cc
	../../../src/render/gl/gl_render.cc
	../../../src/render/gl/gl_shader.cc
	../../../src/render/gl/glsl_shaders.cc
	../../../src/ui/app.cc
	../../../src/ui/screen.cc
	../../../src/ui/event.cc
	../../../src/ui/pre_render.cc
	../../../src/ui/window.cc
	../../../src/ui/keyboard.cc
	../../../src/ui/view_prop.cc
	../../../src/ui/filter.cc
	../../../src/ui/layer.cc
	../../../src/ui/draw.cc
	../../../src/ui/css/css_sheets.cc
	../../../src/ui/css/css_class.cc
	../../../src/ui/css/css_prop.cc
	../../../src/ui/action/action.cc
	../../../src/ui/action/center.cc
	../../../src/ui/action/group.cc
	../../../src/ui/action/keyframe.cc
	../../../src/ui/text/text_blob.cc
	../../../src/ui/text/text_lines.cc
	../../../src/ui/text/text_opts.cc
	../../../src/ui/view/box.cc
	../../../src/ui/view/free.cc
	../../../src/ui/view/flex.cc
	../../../src/ui/view/flow.cc
	../../../src/ui/view/image.cc
	../../../src/ui/view/input.cc
	../../../src/ui/view/textarea.cc
	../../../src/ui/view/label.cc
	../../../src/ui/view/root.cc
	../../../src/ui/view/scroll.cc
	../../../src/ui/view/text.cc
	../../../src/ui/view/button.cc
	../../../src/ui/view/view.cc
	../../../src/ui/view/matrix.cc
)

# add_library(quark-js SHARED
# 	../../../out/native-inl-js.cc
# 	../../../out/native-lib-js.cc
# 	../../../out/native-ext-js.cc
# 	../../../quark-js/js-cls.cc
# 	../../../quark-js/js.cc
# 	../../../quark-js/quarks.cc
# 	../../../quark-js/str.cc
# 	../../../quark-js/v8.cc
# 	../../../quark-js/wrap.cc
# 	../../../quark-js/node.cc
# 	# binding quark-util
# 	../../../quark-js/binding/cb.cc
# 	../../../quark-js/binding/fs.cc
# 	../../../quark-js/binding/fs-reader.cc
# 	../../../quark-js/binding/fs-path.cc
# 	../../../quark-js/binding/http.cc
# 	../../../quark-js/binding/util.cc
# 	../../../quark-js/binding/storage.cc
# 	../../../quark-js/binding/json.cc
# 	../../../quark-js/binding/event.cc
# 	../../../quark-js/binding/sys.cc
# 	../../../quark-js/binding/event.cc
# 	../../../quark-js/binding/timer.cc
# 	../../../quark-js/binding/console.cc
# 	../../../quark-js/binding/buffer.cc
# 	# binding quark-gui
# 	../../../quark-js/binding/value.cc
# 	../../../quark-js/binding/binding.cc
# 	../../../quark-js/binding/action.cc
# 	../../../quark-js/binding/action-frame.cc
# 	../../../quark-js/binding/app.cc
# 	../../../quark-js/binding/audio-player.cc
# 	../../../quark-js/binding/video.cc
# 	../../../quark-js/binding/media.cc
# 	../../../quark-js/binding/div.cc
# 	../../../quark-js/binding/display-port.cc
# 	../../../quark-js/binding/indep.cc
# 	../../../quark-js/binding/image.cc
# 	../../../quark-js/binding/layout.cc
# 	../../../quark-js/binding/box.cc
# 	../../../quark-js/binding/view.cc
# 	../../../quark-js/binding/root.cc
# 	../../../quark-js/binding/span.cc
# 	../../../quark-js/binding/sprite.cc
# 	../../../quark-js/binding/hybrid.cc
# 	../../../quark-js/binding/text-font.cc
# 	../../../quark-js/binding/text-node.cc
# 	../../../quark-js/binding/label.cc
# 	../../../quark-js/binding/limit.cc
# 	../../../quark-js/binding/panel.cc
# 	../../../quark-js/binding/button.cc
# 	../../../quark-js/binding/scroll.cc
# 	../../../quark-js/binding/css.cc
# 	../../../quark-js/binding/font.cc
# 	../../../quark-js/binding/text.cc
# 	../../../quark-js/binding/input.cc
# 	../../../quark-js/binding/background.cc
# )

# add_library(quark-trial SHARED
# 	../../../trial/fs-search.cc
# 	../../../trial/jsx.cc
# )

# add_library(quark-test SHARED
# 	../../../test/test.cc
# 	#../../../test/test-quarks.cc
# 	../../../test/test-fs.cc
# 	../../../test/test-fs2.cc
# 	../../../test/test-gui.cc
# 	../../../test/test-freetype.cc
# 	../../../test/test-json.cc
# 	../../../test/test-string.cc
# 	../../../test/test-list.cc
# 	../../../test/test-map.cc
# 	../../../test/test-event.cc
# 	../../../test/test-zlib.cc
# 	../../../test/test-http.cc
# 	../../../test/test-http2.cc
# 	../../../test/test-http3.cc
# 	../../../test/test-https.cc
# 	../../../test/test-thread.cc
# 	../../../test/test-ffmpeg.cc
# 	../../../test/test-number.cc
# 	../../../test/test-uv.cc
# 	../../../test/test-net.cc
# 	../../../test/test-fs-async.cc
# 	../../../test/test-ssl.cc
# 	../../../test/test-net-ssl.cc
# 	../../../test/test-http-cookie.cc
# 	../../../test/test-localstorage.cc
# 	../../../test/test-buffer.cc
# 	../../../test/test-demo.cc
# 	../../../test/test-jsc.cc
# 	../../../test/test-v8.cc
# 	../../../test/test-loop.cc
# 	../../../test/test-sys.cc
# 	../../../test/test-mutex.cc
# 	../../../test/test-ios-run-loop.cc
# 	../../../test/test-benchmark.cc
# 	../../../test/test-sizeof.cc
# 	../../../test/test-util.cc
# 	../../../test/test-alsa-ff.cc
# 	../../../test/test-linux-input.cc
# 	../../../test/test-linux-input-2.cc
# )

target_link_libraries(quark       atomic z android log OpenSLES GLESv3 EGL mediandk quark_deps_test)
# target_link_libraries(quark-media quark OpenSLES GLESv3 EGL mediandk quark-depes-test)
# target_link_libraries(quark-js    quark quark-depes-test)
# target_link_libraries(quark-trial quark quark-depes-test)
# target_link_libraries(quark-test  quark quark-js quark-trial quark-depes-test)
