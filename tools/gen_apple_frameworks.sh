#!/bin/sh

base=$(dirname $0)
cd $base/..
out=$1
os=$2
gen=./tools/gen_apple_framework.js

gen_framework() {
	arr=
	for i in $4; do
		arr=$arr"out/$os."$i".Release/lib"$1".dylib "
	done
	node $gen $os $1 "no-cut" "$2" $out/product/$os/Frameworks/$3 $arr
}

if [ $os = "ios" ]; then
	gen_framework quark  src  iphoneos          "arm64"
	gen_framework quark  src  iphonesimulator   "arm64.emulator x64.emulator"
else
	gen_framework quark  src  macosx            "arm64 x64"
fi
