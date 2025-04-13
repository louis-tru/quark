The commands of search libraries files and includes files
======================

ld --verbose | grep SEARCH_DIR | tr -s ' ;' \\012
ldconfig -p| grep fontconfig
ldd test1
nm -D /usr/lib/aarch64-linux-gnu/libc.so.6 | grep 'A GLIBC_'

echo "#include <stdio.h>" | gcc -fsyntax-only -v -xc -H -
echo "#include <fontconfig/fontconfig.h>" | gcc -fsyntax-only -v -xc -H -
syntax-gcc '#include <fontconfig/fontconfig.h>'

valgrind --tool=memcheck --leak-check=full --show-reachable=yes --trace-children=yes ./leak

sudo dpkg --add-architecture arm64
sudo dpkg --add-architecture armhf

‌Ubuntu 18.04使用的是GLIBC2.27
‌Ubuntu 20.04使用的是GLIBC2.31‌‌
‌Ubuntu 22.04使用的是GLIBC2.35‌‌
‌Ubuntu 24.04使用的是GLIBC2.39‌