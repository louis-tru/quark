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

# lutils
link_dylib lutils "$obj/lutils $obj/libuv $obj/openssl $obj/http_parser " \
	"-lminizip -lbplus -lz " "-framework Foundation -framework UIKit "
framework lutils $out/../../lutils

# langou
link_dylib langou "$obj/lutils $obj/langou $obj/libuv $obj/openssl $obj/http_parser " \
	"-lreachability -ltess2 -lft2 -ltinyxml2 -liconv -lbz2 " \
	"-framework Foundation -framework SystemConfiguration -framework OpenGLES \
	-framework CoreGraphics -framework QuartzCore -framework UIKit \
	-framework MessageUI -framework lutils "
# gen temp framework
framework langou

# langou-media
link_dylib langou-media "$obj/langou-media" "-liconv -lbz2 -lz -lFFmpeg" \
	"-framework AudioToolbox -framework CoreVideo -framework VideoToolbox \
	-framework CoreMedia -framework lutils -framework langou"
framework langou-media no-inc

# langou-v8
if [ "$use_v8_link" = "1" ]; then
	link_dylib langou-v8 "$obj/v8-link" "" "-framework JavaScriptCore"
else
	# $obj/v8_base/depe/node/deps/v8/src/api.o
	# $obj/v8_base/depe/node/deps/v8/src/inspector
	link_dylib langou-v8 "$obj/v8_base $obj/v8_libplatform" \
		"-lv8_base -lv8_libbase -lv8_libsampler -lv8_builtins_setup \
		-lv8_nosnapshot -lv8_builtins_generators" ""
fi
framework langou-v8 $out/../../depe/v8-link/include

# langou-js
link_dylib langou-js "$obj/langou-js" "" \
	"-framework lutils -framework langou -framework langou-media \
	-framework langou-v8 -framework JavaScriptCore"
framework langou-js no-inc

# langou-node
link_dylib langou-node "$obj/node" "-lnghttp2 -lcares -lz" \
	"-framework lutils -framework langou -framework langou-js -framework langou-v8"
framework langou-node no-inc

