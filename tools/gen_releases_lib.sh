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

cd noproj/product

tar cfvz ../../releases/ios.tar.gz ios
tar cfvz ../../releases/android.tar.gz android
tar cfvz ../../releases/include.tar.gz include

cd ../..

# linux ubuntu shread

mkdir -p linux-ubuntu.x64.Release.shared/bin
mkdir -p linux-ubuntu.x64.Release.shared/lib
cp -rf noproj/product/include linux-ubuntu.x64.Release.shared
scp $host:~/quark/out/linux-ubuntu.x64.Release.shared/quark          linux-ubuntu.x64.Release.shared/bin
scp $host:~/quark/out/linux-ubuntu.x64.Release.shared/lib.target/* linux-ubuntu.x64.Release.shared/lib
tar cfvz releases/linux-ubuntu.x64.Release.shared.tar.gz \
	linux-ubuntu.x64.Release.shared/bin \
	linux-ubuntu.x64.Release.shared/lib linux-ubuntu.x64.Release.shared/include

mkdir -p linux-ubuntu.armv7.Release.shared/bin
mkdir -p linux-ubuntu.armv7.Release.shared/lib
cp -rf noproj/product/include linux-ubuntu.armv7.Release.shared
scp $host:~/quark/out/linux-ubuntu.armv7.Release.shared/quark          linux-ubuntu.armv7.Release.shared/bin
scp $host:~/quark/out/linux-ubuntu.armv7.Release.shared/lib.target/* linux-ubuntu.armv7.Release.shared/lib
tar cfvz releases/linux-ubuntu.armv7.Release.shared.tar.gz \
	linux-ubuntu.armv7.Release.shared/bin \
	linux-ubuntu.armv7.Release.shared/lib linux-ubuntu.armv7.Release.shared/include

# linux ubuntu

mkdir -p linux-ubuntu.x64.Release/bin
cp -rf noproj/product/include linux-ubuntu.x64.Release
scp $host:~/quark/out/linux-ubuntu.x64.Release/quark linux-ubuntu.x64.Release/bin
tar cfvz releases/linux-ubuntu.x64.Release.tar.gz \
	linux-ubuntu.x64.Release/bin linux-ubuntu.x64.Release/include

mkdir -p linux-ubuntu.armv7.Release/bin
cp -rf noproj/product/include linux-ubuntu.armv7.Release
scp $host:~/quark/out/linux-ubuntu.armv7.Release/quark linux-ubuntu.armv7.Release/bin
tar cfvz releases/linux-ubuntu.armv7.Release.tar.gz \
	linux-ubuntu.armv7.Release/bin linux-ubuntu.armv7.Release/include


exit 0