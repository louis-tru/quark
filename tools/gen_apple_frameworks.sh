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

gen_framework quark       quark                  iphonesimulator "x64"
gen_framework quark-media no-inc               iphonesimulator "x64"
gen_framework quark-js    quark-js               iphonesimulator "x64"
gen_framework quark-node  no-inc               iphonesimulator "x64"

gen_framework quark       quark                  iphoneos "arm64" # x64 arm64 armv7
gen_framework quark-media no-inc               iphoneos "arm64"
gen_framework quark-js    quark-js               iphoneos "arm64"
gen_framework quark-node  no-inc               iphoneos "arm64"

gen_framework quark-js    quark-js               iphoneos/Debug "arm64.v8"
gen_framework quark-node  no-inc               iphoneos/Debug "arm64.v8"

cd $out/product/ios/Frameworks/iphoneos
# echo '#undef USE_JSC\n#define USE_JSC 0' > Debug/quark-js.framework/Headers/v8-jsccfg.h
# echo '#undef USE_JSC\n#define USE_JSC 1' > quark-js.framework/Headers/v8-jsccfg.h

cd ../iphonesimulator && mkdir -p Debug Release
# iphonesimulator

cd Debug
[ ! -L quark.framework ] && ln -s ../quark.framework
[ ! -L quark-media.framework ] && ln -s ../quark-media.framework
[ ! -L quark-js.framework ] && ln -s ../quark-js.framework
[ ! -L quark-node.framework ] && ln -s ../quark-node.framework

cd ../Release
[ ! -L quark.framework ] && ln -s ../quark.framework
[ ! -L quark-media.framework ] && ln -s ../quark-media.framework
[ ! -L quark-js.framework ] && ln -s ../quark-js.framework
[ ! -L quark-node.framework ] && ln -s ../quark-node.framework


cd ../../iphoneos && mkdir -p Debug Release
# iphoneos

cd Debug
[ ! -L quark.framework ] && ln -s ../quark.framework
[ ! -L quark-media.framework ] && ln -s ../quark-media.framework

cd ../Release
[ ! -L quark.framework ] && ln -s ../quark.framework
[ ! -L quark-media.framework ] && ln -s ../quark-media.framework
[ ! -L quark-js.framework ] && ln -s ../quark-js.framework
[ ! -L quark-node.framework ] && ln -s ../quark-node.framework

exit 0
