#!/bin/sh

base=$(dirname "$0")
cd $base/..

REMOTE_COMPILE_HOST=$1
config=$2
target=$3
tar=$4

echo config='"'$config'"' > out/remote_build1.sh
cat tools/remote_build1.sh >> out/remote_build1.sh

ssh $REMOTE_COMPILE_HOST 'bash -l -s' < out/remote_build1.sh $target $tar $V
scp $REMOTE_COMPILE_HOST:~/quark/out/remote_build.tgz out

cd out
tar xfvz remote_build.tgz