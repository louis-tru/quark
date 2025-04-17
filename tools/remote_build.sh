#!/bin/sh

target=$1
v=$2

if [ "$v" != "" ]; then
	export V=1
fi

cd quark
make $target -j `nproc`
cd out
tar cfvz remote_build.tgz qkmake/product/$target
