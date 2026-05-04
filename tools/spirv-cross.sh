brew install shaderc spirv-cross

glslc test.vert -o test.vert.spv
glslc --target-env=opengl -fshader-stage=vert test.vert -o test.vert.spv
glslc --target-env=opengl -fshader-stage=frag test.vert -o test.vert.spv
spirv-cross test.vert.spv --msl > test.metal
spirv-cross test.vert.spv --es > test.glsl
spirv-cross test.vert.spv --msl --rename-entry-point main _main vert > test.metal
spirv-cross test.vert.spv --msl --rename-entry-point main qk_vs_main vert > test.vert.metal
spirv-cross test.frag.spv --msl --rename-entry-point main qk_fs_main frag > test.frag.metal


glslc temp.vert -o temp.vert.spv
glslc temp.frag -o temp.frag.spv

spirv-cross temp.vert.spv --msl > temp.vert.msl
spirv-cross temp.frag.spv --msl > temp.frag.msl