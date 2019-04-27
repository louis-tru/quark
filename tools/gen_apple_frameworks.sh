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

gen_framework qgr       ""                   iphonesimulator "x64"
gen_framework qgr-media no-inc               iphonesimulator "x64"
gen_framework qgr-v8    depe/v8-link/include iphonesimulator "x64"
gen_framework qgr-js    no-inc               iphonesimulator "x64"
gen_framework qgr-node  no-inc               iphonesimulator "x64"

gen_framework qgr       ""                   iphoneos "arm64" # x64 arm64 armv7
gen_framework qgr-media no-inc               iphoneos "arm64"
gen_framework qgr-v8    depe/v8-link/include iphoneos "arm64"
gen_framework qgr-js    no-inc               iphoneos "arm64"
gen_framework qgr-node  no-inc               iphoneos "arm64"

gen_framework qgr-v8    depe/v8-link/include iphoneos/Debug "arm64.v8"
gen_framework qgr-js    no-inc               iphoneos/Debug "arm64.v8"
gen_framework qgr-node  no-inc               iphoneos/Debug "arm64.v8"

cd $out/product/ios/Frameworks/iphoneos
echo '#undef USE_JSC\n#define USE_JSC 0' > Debug/qgr-v8.framework/Headers/v8-jsccfg.h
echo '#undef USE_JSC\n#define USE_JSC 1' > qgr-v8.framework/Headers/v8-jsccfg.h


cd ../iphonesimulator && mkdir -p Debug Release
# iphonesimulator

cd Debug
[ ! -L qgr.framework ] && ln -s ../qgr.framework
[ ! -L qgr-media.framework ] && ln -s ../qgr-media.framework
[ ! -L qgr-v8.framework ] && ln -s ../qgr-v8.framework
[ ! -L qgr-js.framework ] && ln -s ../qgr-js.framework
[ ! -L qgr-node.framework ] && ln -s ../qgr-node.framework

cd ../Release
[ ! -L qgr.framework ] && ln -s ../qgr.framework
[ ! -L qgr-media.framework ] && ln -s ../qgr-media.framework
[ ! -L qgr-v8.framework ] && ln -s ../qgr-v8.framework
[ ! -L qgr-js.framework ] && ln -s ../qgr-js.framework
[ ! -L qgr-node.framework ] && ln -s ../qgr-node.framework


cd ../../iphoneos && mkdir -p Debug Release
# iphoneos

cd Debug
[ ! -L qgr.framework ] && ln -s ../qgr.framework
[ ! -L qgr-media.framework ] && ln -s ../qgr-media.framework

cd ../Release
[ ! -L qgr.framework ] && ln -s ../qgr.framework
[ ! -L qgr-media.framework ] && ln -s ../qgr-media.framework
[ ! -L qgr-v8.framework ] && ln -s ../qgr-v8.framework
[ ! -L qgr-js.framework ] && ln -s ../qgr-js.framework
[ ! -L qgr-node.framework ] && ln -s ../qgr-node.framework

exit 0
