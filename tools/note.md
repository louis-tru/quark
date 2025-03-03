The commands of search libraries files and includes files
======================

ld --verbose | grep SEARCH_DIR | tr -s ' ;' \\012
ldconfig -p| grep fontconfig

echo "#include <stdio.h>" | gcc -fsyntax-only -v -xc -H -
echo "#include <fontconfig/fontconfig.h>" | gcc -fsyntax-only -v -xc -H -
syntax-gcc '#include <fontconfig/fontconfig.h>'

valgrind --tool=memcheck --leak-check=full --show-reachable=yes --trace-children=yes    ./leak


使用C++与opengl，如何绘制反走样的2D矢量图像，并且不使用多重采样（MSAA）与距离场（SDF），因为需要考虑性能问题。
还有其它方法吗？比如先绘制原始图像（有锯齿的图像），然后再在上面覆盖一个模糊的图像边缘。
是否可以在原始的图像路径上偏移边1到1.5像素，生成一个新的路径，并且记录新路径顶点与原始路径的距离权重，绘图时权重高的绘制更多的透明度。
透明度权重根本不用计算，向外偏移的顶点使用1，向内使用-1，绘图时0表示最高权重即可。
在着色器内可以使用abs函数，让-1变成1，并且绘图时都启用深度测试，这样就不会重复叠加图像。
