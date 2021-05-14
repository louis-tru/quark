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
		-L$out/obj.target/ffmpeg \
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

# flare
link_dylib flare \
	"$obj/flare-utils $obj/libuv $obj/openssl $obj/http_parser \
			$obj/flare " \
	"-lminizip -lbplus -lz 
			-lreachability -ltess2 -lft2 -ltinyxml2 -liconv -lbz2 " \
	"-framework Foundation -framework SystemConfiguration -framework OpenGLES \
			-framework CoreGraphics -framework QuartzCore -framework UIKit -framework MessageUI "
framework flare no-inc # gen temp framework

# flare-media
link_dylib flare-media \
	"$obj/flare-media" \
	"-liconv -lbz2 -lz -lffmpeg" \
	"-framework AudioToolbox -framework CoreVideo -framework VideoToolbox \
			-framework CoreMedia -framework flare"
framework flare-media no-inc # gen temp framework

# flare-js + flare-v8
if [ "$use_v8_link" = "1" ]; then
	link_dylib flare-js \
		"$obj/v8-link $obj/flare-js" \
		"" \
		"-framework flare -framework flare-media -framework JavaScriptCore"
else
	# $obj/v8_base/deps/node/deps/v8/src/api.o
	# $obj/v8_base/deps/node/deps/v8/src/inspector
	link_dylib flare-js \
		"$obj/v8_base $obj/v8_libplatform $obj/flare-js" \
		"-lv8_base -lv8_libbase -lv8_libsampler -lv8_builtins_setup \
				-lv8_nosnapshot -lv8_builtins_generators" \
		"-framework flare -framework flare-media -framework JavaScriptCore"
fi
framework flare-js no-inc # gen temp framework

# flare-node
link_dylib flare-node \
	"$obj/node" \
	"-lnghttp2 -lcares -lz" \
	"-framework flare -framework flare-js"
framework flare-node no-inc # gen temp framework
