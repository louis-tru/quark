
ubuntu
======================
# apt install libasound2-dev
# apt install libx11-dev
# apt install libgles2-mesa-dev
# apt install libxcursor-dev
# apt install libfontconfig1-dev
	/usr/bin/ld: cannot find -lGLESv2
	/usr/bin/ld: cannot find -lEGL
	/usr/bin/ld: cannot find -lX11
	/usr/bin/ld: cannot find -lXi
	/usr/bin/ld: cannot find -lXcursor
	/usr/bin/ld: cannot find -lasound
	/usr/bin/ld: cannot find -lfontconfig
	/usr/bin/ld: cannot find -lz
	/usr/bin/ld: cannot find -lbz2

# apt install systemtap-sdt-dev
	* dtrace
# apt install autoconf
	* autoconf

# apt install g++
# apt install g++-arm-linux-gnueabi
# apt install g++-arm-linux-gnueabihf
# apt install g++-aarch64-linux-gnu

# armv7-a
# ln -s /lib/arm-linux-gnueabihf/libz.so.1 libz.so
# ln -s /lib/arm-linux-gnueabihf/libbz2.so.1 libbz2.so
# ln -s /usr/lib/arm-linux-gnueabihf/libEGL.so.1 libEGL.so
# ln -s /usr/lib/arm-linux-gnueabihf/libGLESv2.so.2 libGLESv2.so
# ln -s /usr/lib/arm-linux-gnueabihf/libX11.so.6 libX11.so
# ln -s /usr/lib/arm-linux-gnueabihf/libXi.so.6 libXi.so
# ln -s /usr/lib/arm-linux-gnueabihf/libasound.so.2 libasound.so

# x86-64
# ln -s /lib/x86_64-linux-gnu/libz.so.1 libz.so
# ln -s /lib/x86_64-linux-gnu/libbz2.so.1 libbz2.so
# ln -s /usr/lib/x86_64-linux-gnu/libEGL.so.1 libEGL.so
# ln -s /usr/lib/x86_64-linux-gnu/libGLESv2.so.2 libGLESv2.so
# ln -s /usr/lib/x86_64-linux-gnu/libX11.so.6 libX11.so
# ln -s /usr/lib/x86_64-linux-gnu/libXi.so.6 libXi.so
# ln -s /usr/lib/x86_64-linux-gnu/libasound.so.2 libasound.so

debian
===========

# https://www.debian.org/distrib/packages
