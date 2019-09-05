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

gen_framework lutils       lutils               iphonesimulator "x64"
gen_framework langou       ""                   iphonesimulator "x64"
gen_framework langou-media no-inc               iphonesimulator "x64"
gen_framework langou-v8    depe/v8-link/include iphonesimulator "x64"
gen_framework langou-js    no-inc               iphonesimulator "x64"
gen_framework langou-node  no-inc               iphonesimulator "x64"

gen_framework lutils       lutils               iphoneos "arm64"
gen_framework langou       ""                   iphoneos "arm64" # x64 arm64 armv7
gen_framework langou-media no-inc               iphoneos "arm64"
gen_framework langou-v8    depe/v8-link/include iphoneos "arm64"
gen_framework langou-js    no-inc               iphoneos "arm64"
gen_framework langou-node  no-inc               iphoneos "arm64"

gen_framework langou-v8    depe/v8-link/include iphoneos/Debug "arm64.v8"
gen_framework langou-js    no-inc               iphoneos/Debug "arm64.v8"
gen_framework langou-node  no-inc               iphoneos/Debug "arm64.v8"

cd $out/product/ios/Frameworks/iphoneos
echo '#undef USE_JSC\n#define USE_JSC 0' > Debug/langou-v8.framework/Headers/v8-jsccfg.h
echo '#undef USE_JSC\n#define USE_JSC 1' > langou-v8.framework/Headers/v8-jsccfg.h


cd ../iphonesimulator && mkdir -p Debug Release
# iphonesimulator

cd Debug
[ ! -L lutils.framework ] && ln -s ../lutils.framework
[ ! -L langou.framework ] && ln -s ../langou.framework
[ ! -L langou-media.framework ] && ln -s ../langou-media.framework
[ ! -L langou-v8.framework ] && ln -s ../langou-v8.framework
[ ! -L langou-js.framework ] && ln -s ../langou-js.framework
[ ! -L langou-node.framework ] && ln -s ../langou-node.framework

cd ../Release
[ ! -L lutils.framework ] && ln -s ../lutils.framework
[ ! -L langou.framework ] && ln -s ../langou.framework
[ ! -L langou-media.framework ] && ln -s ../langou-media.framework
[ ! -L langou-v8.framework ] && ln -s ../langou-v8.framework
[ ! -L langou-js.framework ] && ln -s ../langou-js.framework
[ ! -L langou-node.framework ] && ln -s ../langou-node.framework


cd ../../iphoneos && mkdir -p Debug Release
# iphoneos

cd Debug
[ ! -L lutils.framework ] && ln -s ../lutils.framework
[ ! -L langou.framework ] && ln -s ../langou.framework
[ ! -L langou-media.framework ] && ln -s ../langou-media.framework

cd ../Release
[ ! -L lutils.framework ] && ln -s ../lutils.framework
[ ! -L langou.framework ] && ln -s ../langou.framework
[ ! -L langou-media.framework ] && ln -s ../langou-media.framework
[ ! -L langou-v8.framework ] && ln -s ../langou-v8.framework
[ ! -L langou-js.framework ] && ln -s ../langou-js.framework
[ ! -L langou-node.framework ] && ln -s ../langou-node.framework

exit 0
