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
}

framework() {
	name=$1
	inc="$2"
	node ../../tools/gen_apple_framework.js ios $name "no-cut" "$inc" . ./lib$name.dylib
}

# qgr
link_dylib qgr "$obj/qgr-utils $obj/qgr $obj/libuv $obj/openssl $obj/http_parser " \
	"-lminizip -lreachability -ltess2 -lft2 -ltinyxml2 -liconv -lbz2 -lsqlite3 -lz" \
	"-framework Foundation -framework SystemConfiguration -framework OpenGLES \
	-framework CoreGraphics -framework CoreGraphics -framework UIKit -framework QuartzCore \
	-framework MessageUI "
# gen temp framework
framework qgr

# qgr-media
link_dylib qgr-media "$obj/qgr-media" "-liconv -lbz2 -lz -lFFmpeg" \
	"-framework AudioToolbox -framework CoreVideo -framework VideoToolbox \
	-framework CoreMedia -framework qgr"
framework qgr-media no-inc

# qgr-v8
if [ "$use_v8_link" = "1" ]; then
	link_dylib qgr-v8 "$obj/v8-link" "" "-framework JavaScriptCore"
else
	link_dylib qgr-v8 \
		"$obj/v8_base $obj/v8_libplatform" \
		"-lv8_libbase -lv8_builtins_generators -lv8_libsampler -lv8_builtins_setup -lv8_nosnapshot" ""
fi
framework qgr-v8 $out/../../depe/v8-link/include

# qgr-js
link_dylib qgr-js "$obj/qgr-js" "" "-framework qgr -framework qgr-media \
	-framework qgr-v8 -framework JavaScriptCore"
framework qgr-js no-inc

# qgr-node
link_dylib qgr-node "$obj/node" "-lnghttp2 -lcares -lz" \
	"-framework qgr -framework qgr-js -framework qgr-v8"
framework qgr-node no-inc

