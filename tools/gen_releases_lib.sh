#!/bin/sh

base=$(dirname $0)
cd $base/../out

host="192.168.0.115"

if [ "$REMOTE_COMPILE_HOST" ]; then
	host=$REMOTE_COMPILE_HOST
fi
if [ "$1" ]; then
	host="$1"
fi

mkdir -p releases

cd lmake/product

tar cfvz ../../releases/ios.tar.gz ios
tar cfvz ../../releases/android.tar.gz android
tar cfvz ../../releases/include.tar.gz include

cd ../..

# linux ubuntu shread

mkdir -p linux-ubuntu.x64.Release.shared/bin
mkdir -p linux-ubuntu.x64.Release.shared/lib
cp -rf lmake/product/include linux-ubuntu.x64.Release.shared
scp $host:~/langou/out/linux-ubuntu.x64.Release.shared/langou          linux-ubuntu.x64.Release.shared/bin
scp $host:~/langou/out/linux-ubuntu.x64.Release.shared/lib.target/* linux-ubuntu.x64.Release.shared/lib
tar cfvz releases/linux-ubuntu.x64.Release.shared.tar.gz \
	linux-ubuntu.x64.Release.shared/bin \
	linux-ubuntu.x64.Release.shared/lib linux-ubuntu.x64.Release.shared/include

mkdir -p linux-ubuntu.armv7.Release.shared/bin
mkdir -p linux-ubuntu.armv7.Release.shared/lib
cp -rf lmake/product/include linux-ubuntu.armv7.Release.shared
scp $host:~/langou/out/linux-ubuntu.armv7.Release.shared/langou          linux-ubuntu.armv7.Release.shared/bin
scp $host:~/langou/out/linux-ubuntu.armv7.Release.shared/lib.target/* linux-ubuntu.armv7.Release.shared/lib
tar cfvz releases/linux-ubuntu.armv7.Release.shared.tar.gz \
	linux-ubuntu.armv7.Release.shared/bin \
	linux-ubuntu.armv7.Release.shared/lib linux-ubuntu.armv7.Release.shared/include

# linux ubuntu

mkdir -p linux-ubuntu.x64.Release/bin
cp -rf lmake/product/include linux-ubuntu.x64.Release
scp $host:~/langou/out/linux-ubuntu.x64.Release/langou linux-ubuntu.x64.Release/bin
tar cfvz releases/linux-ubuntu.x64.Release.tar.gz \
	linux-ubuntu.x64.Release/bin linux-ubuntu.x64.Release/include

mkdir -p linux-ubuntu.armv7.Release/bin
cp -rf lmake/product/include linux-ubuntu.armv7.Release
scp $host:~/langou/out/linux-ubuntu.armv7.Release/langou linux-ubuntu.armv7.Release/bin
tar cfvz releases/linux-ubuntu.armv7.Release.tar.gz \
	linux-ubuntu.armv7.Release/bin linux-ubuntu.armv7.Release/include


exit 0