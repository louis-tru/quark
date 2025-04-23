target=$1
tar=$2
v=$3

if [ "$v" != "" ]; then
	export V=1
fi

cd quark

if [ "$config" != "" ]; then
	echo ./configure $config
	./configure $config
fi

make $target -j `nproc`
cd out
if [ "$tar" != "" ]; then
	tar cfvz remote_build.tgz qkmake/product/$tar
else
	touch remote_build.tgz
fi
