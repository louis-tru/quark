
# cmake_minimum_required(VERSION 3.4.1)
cmake_minimum_required(VERSION 3.22.1)
project("quark")

include_directories(../../..)
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
    -DQk_ENABLE_GL
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
add_definitions(-DUSE_JS)

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
	../../../src/ui/view/video.cc
	# js
	../../../src/js/cb.cc
	../../../src/js/init.cc
	../../../src/js/js.cc
	../../../src/js/cls.cc
	../../../src/js/mix.cc
	../../../src/js/api/action.cc
	../../../src/js/api/box.cc
	../../../src/js/api/buffer.cc
	../../../src/js/api/css.cc
	../../../src/js/api/event.cc
	../../../src/js/api/font.cc
	../../../src/js/api/filter.cc
	../../../src/js/api/fs.cc
	../../../src/js/api/http.cc
	../../../src/js/api/os.cc
	../../../src/js/api/player.cc
	../../../src/js/api/screen.cc
	../../../src/js/api/scroll.cc
	../../../src/js/api/storage.cc
	../../../src/js/api/text.cc
	../../../src/js/api/matrix.cc
	../../../src/js/api/types.cc
	../../../src/js/api/ui.cc
	../../../src/js/api/view.cc
	../../../src/js/api/window.cc
	../../../src/js/v8/inspector_agent.cc
	../../../src/js/v8/inspector_io.cc
	../../../src/js/v8/inspector_socket_server.cc
	../../../src/js/v8/inspector_socket.cc
	../../../src/js/v8/v8js.cc
	../../../src/js/v8/v8cls.cc
	../../../out/native-inl-js.cc
	../../../out/native-lib-js.cc
)

add_library(quark_trial SHARED
	../../../trial/fs-search.cc
	../../../trial/jsx.cc
)

add_library(quark_test SHARED
	../../util/test-event.cc
	../../util/test-fs-async.cc
	../../util/test-fs.cc
	../../util/test-fs2.cc
	../../util/test-buffer.cc
	../../util/test-http-cookie.cc
	../../util/test-http.cc
	../../util/test-http2.cc
	../../util/test-http3.cc
	../../util/test-https.cc
	../../util/test-ios-run-loop.cc
	../../util/test-json.cc
	../../util/test-list.cc
	../../util/test-localstorage.cc
	../../util/test-loop.cc
	../../util/test-map.cc
	../../util/test-mutex.cc
	../../util/test-net-ssl.cc
	../../util/test-net.cc
	../../util/test-number.cc
	../../util/test-os-info.cc
	../../util/test-sizeof.cc
	../../util/test-ssl.cc
	../../util/test-string.cc
	../../util/test-thread.cc
	../../util/test-util.cc
	../../util/test-uv.cc
	../../util/test-zlib.cc
	../../util/test-atomic.cc
	../../test-freetype.cc
	../../test-gui.cc
	../../test-alsa-ff.cc
	../../test-ffmpeg.cc
	../../test-media.cc
	../../test-layout.cc
	../../test-canvas.cc
	../../test-rrect.cc
	../../test-draw-efficiency.cc
	../../test-blur.cc
	../../test-subcanvas.cc
	../../test-outimg.cc
	../../test-linux-input.cc
	../../test-css.cc
	../../test-action.cc
	../../test-openurl.cc
	../../test-input.cc
	../../test-jsx.cc
	../../test-v8.cc
	../../test-jsc.cc
	../../test-jsapi.cc
	../../test.cc
)

target_link_libraries(quark       atomic z android log OpenSLES GLESv3 EGL mediandk quark_deps_test)
target_link_libraries(quark_trial quark quark_deps_test)
target_link_libraries(quark_test  quark quark_trial quark_deps_test)
