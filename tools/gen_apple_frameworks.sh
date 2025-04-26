#!/bin/sh

base=$(dirname $0)
cd $base/..
out=$1
os=$2
gen=./tools/gen_apple_framework.js

gen_framework() {
	arr=
	for i in $4; do
		arr=$arr"out/$os."$i".dylib "
	done
	node $gen $os $1 "no-cut" "$2" $out/product/$os/Frameworks/$3 $arr
}

if [ $os = "ios" ]; then
	gen_framework quark  src  iphoneos/Release        "arm64.Release/libquark"
	gen_framework quark  src  iphoneos/Debug          "arm64.Release/libquark.v8"
	gen_framework quark  src  iphonesimulator/Release "arm64.emulator.Release/libquark.v8 x64.emulator.Release/libquark"
	cd $out/product/ios/Frameworks/iphonesimulator
	ln -s Release Debug
else
	gen_framework quark  src  macosx                  "arm64.Release/libquark.v8 x64.Release/libquark"
fi
