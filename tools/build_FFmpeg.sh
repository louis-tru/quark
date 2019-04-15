#!/bin/sh

OBJS_DIR=$INSTALL_DIR/objs

if [ ! -d $OBJS_DIR ] || [ ! -f $PRODUCT_PATH ]; then
	rm -rf $INSTALL_DIR/*
	mkdir -p $INSTALL_DIR
	mkdir $OBJS_DIR
	make install -j2 || exit 1

	OBJS=`find libavutil libavformat libswresample libavcodec compat -name *.o|xargs`

	for OBJ in $OBJS
	do
		echo Copy $OBJ
		DIR=$OBJS_DIR/`dirname $OBJ`
		mkdir -p $DIR
		cp -rf $OBJ $DIR
	done
fi

$AR rc $PRODUCT_PATH `find $OBJS_DIR -name *.o|xargs`
$RANLIB $PRODUCT_PATH