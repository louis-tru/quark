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

# ftr
link_dylib ftr \
	"$obj/ftr-utils $obj/libuv $obj/openssl $obj/http_parser \
			$obj/ftr " \
	"-lminizip -lbplus -lz 
			-lreachability -ltess2 -lft2 -ltinyxml2 -liconv -lbz2 " \
	"-framework Foundation -framework SystemConfiguration -framework OpenGLES \
			-framework CoreGraphics -framework QuartzCore -framework UIKit -framework MessageUI "
framework ftr no-inc # gen temp framework

# ftr-media
link_dylib ftr-media \
	"$obj/ftr-media" \
	"-liconv -lbz2 -lz -lffmpeg" \
	"-framework AudioToolbox -framework CoreVideo -framework VideoToolbox \
			-framework CoreMedia -framework ftr"
framework ftr-media no-inc # gen temp framework

# ftr-js + ftr-v8
if [ "$use_v8_link" = "1" ]; then
	link_dylib ftr-js \
		"$obj/v8-link $obj/ftr-js" \
		"" \
		"-framework ftr -framework ftr-media -framework JavaScriptCore"
else
	# $obj/v8_base/depe/node/deps/v8/src/api.o
	# $obj/v8_base/depe/node/deps/v8/src/inspector
	link_dylib ftr-js \
		"$obj/v8_base $obj/v8_libplatform $obj/ftr-js" \
		"-lv8_base -lv8_libbase -lv8_libsampler -lv8_builtins_setup \
				-lv8_nosnapshot -lv8_builtins_generators" \
		"-framework ftr -framework ftr-media -framework JavaScriptCore"
fi
framework ftr-js no-inc # gen temp framework

# ftr-node
link_dylib ftr-node \
	"$obj/node" \
	"-lnghttp2 -lcares -lz" \
	"-framework ftr -framework ftr-js"
framework ftr-node no-inc # gen temp framework
