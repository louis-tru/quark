#!/bin/sh

skia_source=$1
skia_out=$2
skia_install_dir=$3
# skia_product_path=$4

# python tools/git-sync-deps
# ./bin/gn gen out --args='is_debug=false is_official_build=false is_component_build=false'
ninja -v -C $skia_out

mkdir -p $skia_install_dir

cp -f $skia_out/libskia.a $skia_install_dir/

cd $skia_install_dir

rm -f skia

ln -s $skia_source/include skia
