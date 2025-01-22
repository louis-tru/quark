The commands of search libraries files and includes files
======================
# ld --verbose | grep SEARCH_DIR | tr -s ' ;' \\012
# ldconfig -p| grep fontconfig
# echo "#include <stdio.h>" | gcc -fsyntax-only -v -xc -H -
