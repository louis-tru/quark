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
rm -f skia
ln -s $source/include skia