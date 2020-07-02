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

gen_framework ftr       ftr                  iphonesimulator "x64"
gen_framework ftr-media no-inc               iphonesimulator "x64"
gen_framework ftr-js    ftr-js               iphonesimulator "x64"
gen_framework ftr-node  no-inc               iphonesimulator "x64"

gen_framework ftr       ftr                  iphoneos "arm64" # x64 arm64 armv7
gen_framework ftr-media no-inc               iphoneos "arm64"
gen_framework ftr-js    ftr-js               iphoneos "arm64"
gen_framework ftr-node  no-inc               iphoneos "arm64"

gen_framework ftr-js    ftr-js               iphoneos/Debug "arm64.v8"
gen_framework ftr-node  no-inc               iphoneos/Debug "arm64.v8"

cd $out/product/ios/Frameworks/iphoneos
# echo '#undef USE_JSC\n#define USE_JSC 0' > Debug/ftr-js.framework/Headers/v8-jsccfg.h
# echo '#undef USE_JSC\n#define USE_JSC 1' > ftr-js.framework/Headers/v8-jsccfg.h

cd ../iphonesimulator && mkdir -p Debug Release
# iphonesimulator

cd Debug
[ ! -L ftr.framework ] && ln -s ../ftr.framework
[ ! -L ftr-media.framework ] && ln -s ../ftr-media.framework
[ ! -L ftr-js.framework ] && ln -s ../ftr-js.framework
[ ! -L ftr-node.framework ] && ln -s ../nxnode.framework

cd ../Release
[ ! -L ftr.framework ] && ln -s ../ftr.framework
[ ! -L ftr-media.framework ] && ln -s ../ftr-media.framework
[ ! -L ftr-js.framework ] && ln -s ../ftr-js.framework
[ ! -L ftr-node.framework ] && ln -s ../ftr-node.framework


cd ../../iphoneos && mkdir -p Debug Release
# iphoneos

cd Debug
[ ! -L ftr.framework ] && ln -s ../ftr.framework
[ ! -L ftr-media.framework ] && ln -s ../ftr-media.framework

cd ../Release
[ ! -L ftr.framework ] && ln -s ../ftr.framework
[ ! -L ftr-media.framework ] && ln -s ../ftr-media.framework
[ ! -L ftr-js.framework ] && ln -s ../ftr-js.framework
[ ! -L ftr-node.framework ] && ln -s ../ftr-node.framework

exit 0
