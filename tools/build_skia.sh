#!/bin/sh

set -e

cd $(dirname $0)/../deps/skia

source=`pwd`

if [ "$1" ]; then
	output=$1
else
	output=out
fi

ninja=ninja

if [ ! `which $ninja` ]; then
	if [ `uname` == "Darwin" ] && [ -f /opt/homebrew/bin/ninja ]; then
		ninja=/opt/homebrew/bin/ninja
	fi
fi

cd $output

if [ "$V" ]; then
	$ninja -v
else
	$ninja
fi

# clang -MD -MF obj/src/gpu/gpu.GrBackendSemaphore.o.d -DNDEBUG -DSK_ENABLE_SKSL -mios-simulator-version-min=10.0 \
# -DSK_ASSUME_GL_ES=1 -DSK_ENABLE_API_AVAILABLE -DSK_GAMMA_APPLY_TO_A8 -DSKIA_IMPLEMENTATION=1 \
# -DSK_GL -I../../../../deps/skia -Wno-attributes -fstrict-aliasing -fPIC -fvisibility=hidden \
# -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator14.4.sdk \
# -arch x86_64 -O3 -std=c++17 -fvisibility-inlines-hidden -stdlib=libc++ -fno-aligned-allocation -fno-exceptions \
# -fno-rtti -c ../../../../deps/skia/src/gpu/GrBackendSemaphore.cpp -o obj/src/gpu/gpu.GrBackendSemaphore.o 

# clang -arch x86_64  -mios-simulator-version-min=10.0  -I. -I./ 
# --sysroot=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator14.4.sdk 
# -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator14.4.sdk 
# -D_ISOC99_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -I./compat/dispatch_semaphore -DPIC -DZLIB_CONST -DHAVE_AV_CONFIG_H 
# -std=c99 -mdynamic-no-pic -fomit-frame-pointer -fPIC -pthread -Wdeclaration-after-statement -Wall -Wdisabled-optimization 
# -Wpointer-arith -Wredundant-decls -Wwrite-strings -Wtype-limits -Wundef -Wmissing-prototypes -Wno-pointer-to-int-cast 
# -Wstrict-prototypes -Wempty-body -Wno-parentheses -Wno-switch -Wno-format-zero-length -Wno-pointer-sign -Wno-unused-const-variable 
# -Os -fno-math-errno -fno-signed-zeros -mstack-alignment=16 -Qunused-arguments -Werror=implicit-function-declaration -Werror=missing-prototypes 
# -Werror=return-type  -MMD -MF libavformat/metadata.d -MT libavformat/metadata.o -c -o libavformat/metadata.o libavformat/metadata.c

# rm -f skia
# ln -s $source/include skia

