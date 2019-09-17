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

gen_framework nutils    nutils              iphonesimulator "x64"
gen_framework ngui       ""                   iphonesimulator "x64"
gen_framework ngui-media no-inc               iphonesimulator "x64"
gen_framework ngui-v8    depe/v8-link/include iphonesimulator "x64"
gen_framework ngui-js    no-inc               iphonesimulator "x64"
gen_framework ngui-node  no-inc               iphonesimulator "x64"

gen_framework nutils    nutils              iphoneos "arm64"
gen_framework ngui       ""                   iphoneos "arm64" # x64 arm64 armv7
gen_framework ngui-media no-inc               iphoneos "arm64"
gen_framework ngui-v8    depe/v8-link/include iphoneos "arm64"
gen_framework ngui-js    no-inc               iphoneos "arm64"
gen_framework ngui-node  no-inc               iphoneos "arm64"

gen_framework ngui-v8    depe/v8-link/include iphoneos/Debug "arm64.v8"
gen_framework ngui-js    no-inc               iphoneos/Debug "arm64.v8"
gen_framework ngui-node  no-inc               iphoneos/Debug "arm64.v8"

cd $out/product/ios/Frameworks/iphoneos
echo '#undef USE_JSC\n#define USE_JSC 0' > Debug/ngui-v8.framework/Headers/v8-jsccfg.h
echo '#undef USE_JSC\n#define USE_JSC 1' > ngui-v8.framework/Headers/v8-jsccfg.h


cd ../iphonesimulator && mkdir -p Debug Release
# iphonesimulator

cd Debug
[ ! -L nutils.framework ] && ln -s ../nutils.framework
[ ! -L ngui.framework ] && ln -s ../ngui.framework
[ ! -L ngui-media.framework ] && ln -s ../ngui-media.framework
[ ! -L ngui-v8.framework ] && ln -s ../ngui-v8.framework
[ ! -L ngui-js.framework ] && ln -s ../ngui-js.framework
[ ! -L ngui-node.framework ] && ln -s ../ngui-node.framework

cd ../Release
[ ! -L nutils.framework ] && ln -s ../nutils.framework
[ ! -L ngui.framework ] && ln -s ../ngui.framework
[ ! -L ngui-media.framework ] && ln -s ../ngui-media.framework
[ ! -L ngui-v8.framework ] && ln -s ../ngui-v8.framework
[ ! -L ngui-js.framework ] && ln -s ../ngui-js.framework
[ ! -L ngui-node.framework ] && ln -s ../ngui-node.framework


cd ../../iphoneos && mkdir -p Debug Release
# iphoneos

cd Debug
[ ! -L nutils.framework ] && ln -s ../nutils.framework
[ ! -L ngui.framework ] && ln -s ../ngui.framework
[ ! -L ngui-media.framework ] && ln -s ../ngui-media.framework

cd ../Release
[ ! -L nutils.framework ] && ln -s ../nutils.framework
[ ! -L ngui.framework ] && ln -s ../ngui.framework
[ ! -L ngui-media.framework ] && ln -s ../ngui-media.framework
[ ! -L ngui-v8.framework ] && ln -s ../ngui-v8.framework
[ ! -L ngui-js.framework ] && ln -s ../ngui-js.framework
[ ! -L ngui-node.framework ] && ln -s ../ngui-node.framework

exit 0
