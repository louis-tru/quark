#!/bin/sh

base=$(dirname $0)
cd $base/..
out=$1
os=$2
gen=./tools/gen_apple_framework.js

gen_framework() {
	arr=
	for i in $4; do
		arr=$arr"out/$os."$i".Release.shared/lib"$1".dylib "
	done
	node $gen $os $1 "no-cut" "$2" $out/product/$os/Frameworks/$3 $arr
}

gen_framework flare       flare                  iphonesimulator "x64"
gen_framework flare-media no-inc               iphonesimulator "x64"
gen_framework flare-js    flare-js               iphonesimulator "x64"
gen_framework flare-node  no-inc               iphonesimulator "x64"

gen_framework flare       flare                  iphoneos "arm64" # x64 arm64 armv7
gen_framework flare-media no-inc               iphoneos "arm64"
gen_framework flare-js    flare-js               iphoneos "arm64"
gen_framework flare-node  no-inc               iphoneos "arm64"

gen_framework flare-js    flare-js               iphoneos/Debug "arm64.v8"
gen_framework flare-node  no-inc               iphoneos/Debug "arm64.v8"

cd $out/product/ios/Frameworks/iphoneos
# echo '#undef USE_JSC\n#define USE_JSC 0' > Debug/flare-js.framework/Headers/v8-jsccfg.h
# echo '#undef USE_JSC\n#define USE_JSC 1' > flare-js.framework/Headers/v8-jsccfg.h

cd ../iphonesimulator && mkdir -p Debug Release
# iphonesimulator

cd Debug
[ ! -L flare.framework ] && ln -s ../flare.framework
[ ! -L flare-media.framework ] && ln -s ../flare-media.framework
[ ! -L flare-js.framework ] && ln -s ../flare-js.framework
[ ! -L flare-node.framework ] && ln -s ../flare-node.framework

cd ../Release
[ ! -L flare.framework ] && ln -s ../flare.framework
[ ! -L flare-media.framework ] && ln -s ../flare-media.framework
[ ! -L flare-js.framework ] && ln -s ../flare-js.framework
[ ! -L flare-node.framework ] && ln -s ../flare-node.framework


cd ../../iphoneos && mkdir -p Debug Release
# iphoneos

cd Debug
[ ! -L flare.framework ] && ln -s ../flare.framework
[ ! -L flare-media.framework ] && ln -s ../flare-media.framework

cd ../Release
[ ! -L flare.framework ] && ln -s ../flare.framework
[ ! -L flare-media.framework ] && ln -s ../flare-media.framework
[ ! -L flare-js.framework ] && ln -s ../flare-js.framework
[ ! -L flare-node.framework ] && ln -s ../flare-node.framework

exit 0
