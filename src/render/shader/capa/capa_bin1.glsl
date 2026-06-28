// CAPA pass 3.1
// Compute the number of 16*2 dispatch groups for the backdrop pass.
// This is done in a separate pass because the number of boundary tiles is not known until after the bin pass.

Qk_CONSTANT(
	uint maxBoundaryTileCount;
);

#import "_capa.glsl"

#comp
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;

layout(binding=1,set=0,std430) buffer CAPAEnvironments {
	CAPAEnvironment value;
} env;

void main() {
	uint boundaryTileCount = min(env.value.boundaryTileCount, pc.maxBoundaryTileCount);
	uint realBoundaryTileCount = boundaryTileCount > 3u ? boundaryTileCount - 3u : 0u;
	env.value.backdropPassGroups_Size16_2 = uvec4((realBoundaryTileCount + 1u) / 2u, 1u, 1u, 0u);
}
