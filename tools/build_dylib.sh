#!/bin/sh

set -e

out=$1
os=$2
arch_name=$3
sysroot=$4
version_min=$5
without_embed_bitcode=$6
use_js=$7
use_v8=$8
emulator=$9
obj=obj.target
name=quark

cd $out

if [ "$without_embed_bitcode" = "0" ]; then
	embed_bitcode="-fembed-bitcode"
else
	embed_bitcode=""
fi

link_dylib() {
	name=$1
	libname=$2
	dirs=$3
	links=$4
	frameworks=$5

	find $dirs -name *.o > $name.LinkFileList

	if [ "$os" = "mac" ]; then
		flags="-mmacosx-version-min=$version_min"
	elif [ "$os" = "ios" ]; then
		if [ "$emulator" = "1" ]; then
			flags="-mios-simulator-version-min=$version_min"
		else
			flags="-miphoneos-version-min=$version_min"
		fi
	fi

	clang++ -arch $arch_name -dynamiclib \
		-isysroot $sysroot \
		-L$out \
		-F$out \
		-L$out/obj.target/ffmpeg \
		-L$sysroot/System/Library/Frameworks \
		-stdlib=libc++ \
		-filelist $name.LinkFileList \
		-o lib$name.dylib \
		-install_name @rpath/$libname.framework/$libname \
		-Xlinker -rpath -Xlinker @executable_path/Frameworks \
		-Xlinker -rpath -Xlinker @loader_path/Frameworks \
		$flags \
		$embed_bitcode \
		-dead_strip \
		-fobjc-arc \
		-fobjc-link-runtime $links $frameworks \
		# -single_module \

	strip -S lib$name.dylib
}

framework() {
	name=$1
	inc="$2"
	node ../../tools/gen_apple_framework.js $os $name "no-cut" "$inc" . ./lib$name.dylib
}

dirs="$obj/quark-util $obj/quark $obj/quark-media"

links="\
	-lbptree \
	-lfreetype -lgif \
	-lhttp_parser -lminizip -lopenssl \
	-lreachability -ltess2 -luv -liconv -lbz2 -lz -lffmpeg \
"

frameworks="\
	-framework Foundation \
	-framework SystemConfiguration \
	-framework CoreGraphics \
	-framework QuartzCore \
	-framework AudioToolbox \
	-framework CoreVideo \
	-framework VideoToolbox \
	-framework CoreMedia \
"

if [ "$os" = "mac" ]; then
	frameworks="$frameworks \
		-framework OpenGL \
		-framework AppKit \
		-framework IOKit \
	"
else # ios
	frameworks="$frameworks \
		-framework OpenGLES \
		-framework UIKit \
		-framework MessageUI \
		-framework CoreText \
	"
fi

if [ "$use_js" = "1" ]; then
	if [ "$use_v8" = "1" ]; then
		name=quark.v8
		rm -rf $obj/quark-js/src/js/jsc
		links="$links -lv8_initializers -lv8_libbase -lv8_libplatform 
			-lv8_libsampler -lv8_snapshot -lv8_base_without_compiler -lv8_compiler"
	else
		rm -rf $obj/quark-js/src/js/v8
		frameworks="$frameworks -framework JavaScriptCore "
	fi
	dirs="$dirs $obj/quark-js "
fi

link_dylib $name quark "$dirs" "$links" "$frameworks"
framework $name no-inc # gen temp framework
