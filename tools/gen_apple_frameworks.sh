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

gen_framework noug       noug                  iphonesimulator "x64"
gen_framework noug-media no-inc               iphonesimulator "x64"
gen_framework noug-js    noug-js               iphonesimulator "x64"
gen_framework noug-node  no-inc               iphonesimulator "x64"

gen_framework noug       noug                  iphoneos "arm64" # x64 arm64 armv7
gen_framework noug-media no-inc               iphoneos "arm64"
gen_framework noug-js    noug-js               iphoneos "arm64"
gen_framework noug-node  no-inc               iphoneos "arm64"

gen_framework noug-js    noug-js               iphoneos/Debug "arm64.v8"
gen_framework noug-node  no-inc               iphoneos/Debug "arm64.v8"

cd $out/product/ios/Frameworks/iphoneos
# echo '#undef USE_JSC\n#define USE_JSC 0' > Debug/noug-js.framework/Headers/v8-jsccfg.h
# echo '#undef USE_JSC\n#define USE_JSC 1' > noug-js.framework/Headers/v8-jsccfg.h

cd ../iphonesimulator && mkdir -p Debug Release
# iphonesimulator

cd Debug
[ ! -L noug.framework ] && ln -s ../noug.framework
[ ! -L noug-media.framework ] && ln -s ../noug-media.framework
[ ! -L noug-js.framework ] && ln -s ../noug-js.framework
[ ! -L noug-node.framework ] && ln -s ../noug-node.framework

cd ../Release
[ ! -L noug.framework ] && ln -s ../noug.framework
[ ! -L noug-media.framework ] && ln -s ../noug-media.framework
[ ! -L noug-js.framework ] && ln -s ../noug-js.framework
[ ! -L noug-node.framework ] && ln -s ../noug-node.framework


cd ../../iphoneos && mkdir -p Debug Release
# iphoneos

cd Debug
[ ! -L noug.framework ] && ln -s ../noug.framework
[ ! -L noug-media.framework ] && ln -s ../noug-media.framework

cd ../Release
[ ! -L noug.framework ] && ln -s ../noug.framework
[ ! -L noug-media.framework ] && ln -s ../noug-media.framework
[ ! -L noug-js.framework ] && ln -s ../noug-js.framework
[ ! -L noug-node.framework ] && ln -s ../noug-node.framework

exit 0
