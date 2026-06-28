// CAPA path-tile initialization pass.
// Initialize every allocated path tile before binning writes boundary/edge data.

Qk_CONSTANT(
	uint maxPathTileCount;
);

#import "_capa.glsl"

#comp
layout(local_size_x=32, local_size_y=1, local_size_z=1) in;

layout(binding=2,set=0,std430) buffer CAPAPathTiles {
	CAPAPathTile values[];
} pathTiles;

void main() {
	// 1 thread per path tile, 32 path tiles per group
	uint pathTileIndex = gl_GlobalInvocationID.x;
	if (pathTileIndex >= pc.maxPathTileCount)
		return;
	pathTiles.values[pathTileIndex] = CAPAPathTile(0u, 0u, CAPA_NIL, CAPA_NIL, 0u);

	// 32 threads per group, 32 path tiles per thread, 1024 path tiles per group
	// uint pathTileIndex = gl_GlobalInvocationID.x * 32u;
	// for (uint i = 0u; i < 32u; i++) {
	// 	uint index = pathTileIndex + i;
	// 	if (index >= pc.maxPathTileCount)
	// 		return;
	// 	pathTiles.values[index] = CAPAPathTile(0u, 0u, CAPA_NIL, CAPA_NIL, 0u);
	// }
}
