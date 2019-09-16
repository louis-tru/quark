#!/bin/sh

out=$1
embed_bitcode=$2
use_v8_link=$3
arch_name=$4
sysroot=$5
version_min=$6
obj=obj.target

cd $out

if [ "$embed_bitcode" = 1 ]; then
	embed_bitcode="-fembed-bitcode"
else
	embed_bitcode=""
fi

link_dylib() {
	name=$1
	dirs=$2
	links=$3
	frameworks=$4

	find $dirs -name *.o > $name.LinkFileList

	clang++ -arch $arch_name -dynamiclib \
		-isysroot $sysroot \
		-L$out \
		-F$out \
		-L$out/obj.target/FFmpeg \
		-L$sysroot/System/Library/Frameworks \
		-stdlib=libc++ \
		-filelist $name.LinkFileList \
		-o lib$name.dylib \
		-single_module \
		-install_name @rpath/$name.framework/$name \
		-Xlinker -rpath -Xlinker @executable_path/Frameworks \
		-Xlinker -rpath -Xlinker @loader_path/Frameworks \
		-miphoneos-version-min=$version_min \
		$embed_bitcode \
		-dead_strip \
		-fobjc-arc \
		-fobjc-link-runtime $links $frameworks

	strip -S lib$name.dylib
}

framework() {
	name=$1
	inc="$2"
	node ../../tools/gen_apple_framework.js ios $name "no-cut" "$inc" . ./lib$name.dylib
}

# niutils
link_dylib niutils "$obj/niutils $obj/libuv $obj/openssl $obj/http_parser " \
	"-lminizip -lbplus -lz " "-framework Foundation -framework UIKit "
framework niutils $out/../../niutils

# ngui
link_dylib ngui "$obj/ngui " \
	"-lreachability -ltess2 -lft2 -ltinyxml2 -liconv -lbz2 " \
	"-framework Foundation -framework SystemConfiguration -framework OpenGLES \
	-framework CoreGraphics -framework QuartzCore -framework UIKit \
	-framework MessageUI -framework niutils "
# gen temp framework
framework ngui

# ngui-media
link_dylib ngui-media "$obj/ngui-media" "-liconv -lbz2 -lz -lFFmpeg" \
	"-framework AudioToolbox -framework CoreVideo -framework VideoToolbox \
	-framework CoreMedia -framework niutils -framework ngui"
framework ngui-media no-inc

# ngui-v8
if [ "$use_v8_link" = "1" ]; then
	link_dylib ngui-v8 "$obj/v8-link" "" "-framework JavaScriptCore"
else
	# $obj/v8_base/depe/node/deps/v8/src/api.o
	# $obj/v8_base/depe/node/deps/v8/src/inspector
	link_dylib ngui-v8 "$obj/v8_base $obj/v8_libplatform" \
		"-lv8_base -lv8_libbase -lv8_libsampler -lv8_builtins_setup \
		-lv8_nosnapshot -lv8_builtins_generators" ""
fi
framework ngui-v8 $out/../../depe/v8-link/include

# ngui-js
link_dylib ngui-js "$obj/ngui-js" "" \
	"-framework niutils -framework ngui -framework ngui-media \
	-framework ngui-v8 -framework JavaScriptCore"
framework ngui-js no-inc

# ngui-node
link_dylib ngui-node "$obj/node" "-lnghttp2 -lcares -lz" \
	"-framework niutils -framework ngui -framework ngui-js -framework ngui-v8"
framework ngui-node no-inc

