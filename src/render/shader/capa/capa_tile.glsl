// CAPA small tile init pass.
// Clear every allocated CAPASmallTile before binning writes short-edge heads.

Qk_CONSTANT(
	uint maxPathTileCount;
);

#import "_capa.glsl"

#comp
layout(local_size_x=32, local_size_y=1, local_size_z=1) in;

layout(binding=2,set=0,std430) buffer CAPASmallTiles {
	CAPASmallTile values[];
} smallTiles;

void main() {
	// 1 thread per small tile, 32 small tiles per group
	uint smallTileIndex = gl_GlobalInvocationID.x;
	if (smallTileIndex >= pc.maxPathTileCount)
		return;
	smallTiles.values[smallTileIndex].value = CAPA_NIL;
}
