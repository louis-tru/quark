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

gen_framework nxkit   nxkit                iphonesimulator "x64"
gen_framework ngui    ""                   iphonesimulator "x64"
gen_framework nxmedia no-inc               iphonesimulator "x64"
gen_framework nxv8    depe/v8-link/include iphonesimulator "x64"
gen_framework nxjs    no-inc               iphonesimulator "x64"
gen_framework nxnode  no-inc               iphonesimulator "x64"

gen_framework nxkit   nxkit                iphoneos "arm64"
gen_framework ngui    ""                   iphoneos "arm64" # x64 arm64 armv7
gen_framework nxmedia no-inc               iphoneos "arm64"
gen_framework nxv8    depe/v8-link/include iphoneos "arm64"
gen_framework nxjs    no-inc               iphoneos "arm64"
gen_framework nxnode  no-inc               iphoneos "arm64"

gen_framework nxv8    depe/v8-link/include iphoneos/Debug "arm64.v8"
gen_framework nxjs    no-inc               iphoneos/Debug "arm64.v8"
gen_framework nxnode  no-inc               iphoneos/Debug "arm64.v8"

cd $out/product/ios/Frameworks/iphoneos
echo '#undef USE_JSC\n#define USE_JSC 0' > Debug/nxv8.framework/Headers/v8-jsccfg.h
echo '#undef USE_JSC\n#define USE_JSC 1' > nxv8.framework/Headers/v8-jsccfg.h


cd ../iphonesimulator && mkdir -p Debug Release
# iphonesimulator

cd Debug
[ ! -L nxkit.framework ] && ln -s ../nxkit.framework
[ ! -L ngui.framework ] && ln -s ../ngui.framework
[ ! -L nxmedia.framework ] && ln -s ../nxmedia.framework
[ ! -L nxv8.framework ] && ln -s ../nxv8.framework
[ ! -L nxjs.framework ] && ln -s ../nxjs.framework
[ ! -L nxnode.framework ] && ln -s ../nxnode.framework

cd ../Release
[ ! -L nxkit.framework ] && ln -s ../nxkit.framework
[ ! -L ngui.framework ] && ln -s ../ngui.framework
[ ! -L nxmedia.framework ] && ln -s ../nxmedia.framework
[ ! -L nxv8.framework ] && ln -s ../nxv8.framework
[ ! -L nxjs.framework ] && ln -s ../nxjs.framework
[ ! -L nxnode.framework ] && ln -s ../nxnode.framework


cd ../../iphoneos && mkdir -p Debug Release
# iphoneos

cd Debug
[ ! -L nxkit.framework ] && ln -s ../nxkit.framework
[ ! -L ngui.framework ] && ln -s ../ngui.framework
[ ! -L nxmedia.framework ] && ln -s ../nxmedia.framework

cd ../Release
[ ! -L nxkit.framework ] && ln -s ../nxkit.framework
[ ! -L ngui.framework ] && ln -s ../ngui.framework
[ ! -L nxmedia.framework ] && ln -s ../nxmedia.framework
[ ! -L nxv8.framework ] && ln -s ../nxv8.framework
[ ! -L nxjs.framework ] && ln -s ../nxjs.framework
[ ! -L nxnode.framework ] && ln -s ../nxnode.framework

exit 0
