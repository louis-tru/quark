#!/bin/sh

CWD=`pwd`
PRODUCT_DIR="out/nxp/product"
BUILD_ALL=$1
BUILD_V8=$2

build() {
	make build
	cd out/ios.$1.Release
	rm -f libngui.a libv8.a
	
	ar rc libngui.a `find obj.target/ngui \
													obj.target/nxkit \
													obj.target/nxjs \
													obj.target/zlib \
													obj.target/ft2 \
													obj.target/http_parser \
													obj.target/openssl \
													obj.target/reachability \
													obj.target/tess2 \
													obj.target/libuv \
													obj.target/tinyxml2 \
													obj.target/FFmpeg/libs \
													-name *.o|xargs`
	ranlib libngui.a

	if [ "$2" = 1 ]; then
		ar rc libv8.a `find obj.target/v8_base \
										obj.target/v8_libbase \
										obj.target/v8_libplatform \
										obj.target/v8_libsampler obj.target/v8_nosnapshot -name *.o|xargs`
		ranlib libv8.a
	fi

	cd $CWD
}

./configure --os=ios --arch=arm
build armv7 $BUILD_V8
./configure --os=ios --arch=arm64
build arm64 $BUILD_V8
./configure --os=ios --arch=x64
build x64 $BUILD_V8

LIBS_NODEUI="out/ios.armv7.Release/libngui.a 
							out/ios.arm64.Release/libngui.a 
							out/ios.x64.Release/libngui.a"
LIBS_V8="out/ios.armv7.Release/libv8.a 
				out/ios.arm64.Release/libv8.a 
				out/ios.x64.Release/libv8.a"

if [ "$BUILD_ALL" = 1 ]; then
	./configure --os=ios --arch=arm --armv7s
	build armv7s $BUILD_V8
	./configure --os=ios --arch=x86
	build x86 $BUILD_V8
	
	LIBS_NODEUI="$OUT_LIBS_NODEUI
								out/ios.armv7s.Release/libngui.a 
								out/ios.x86.Release/libngui.a"
	LIBS_V8="$LIBS_V8 
					out/ios.armv7s.Release/libv8.a 
					out/ios.x86.Release/libv8.a"
fi

mkdir -p ${PRODUCT_DIR}/ios
rm -rf ${PRODUCT_DIR}/ios/*.a

lipo $LIBS_NODEUI -create -output ${PRODUCT_DIR}/ios/libngui.a
if [ "$BUILD_V8" = 1 ]; then
	lipo $LIBS_V8 -create -output ${PRODUCT_DIR}/ios/libv8.a
fi
