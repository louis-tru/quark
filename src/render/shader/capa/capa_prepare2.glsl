// CAPA pass 1.2
// This pass computes the global tile bounds and the number of dispatch groups for the next passes.

Qk_CONSTANT(
	uint maxTaskCount;
	uint maxPathTileCount;
	uint maxPathTileRowCount;
);

#import "_capa.glsl"

#comp
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;

layout(binding=1,set=0,std430) buffer CAPAEnvironments {
	CAPAEnvironment value;
} env;

void main() {
	// write subsequent pass dispatch group counts to the environment
	ivec2 tileSpan = env.value.globalTileBounds.zw - env.value.globalTileBounds.xy;
	if (tileSpan.x > 0 && tileSpan.y > 0) {
		env.value.globalTileSpan = tileSpan;
		env.value.globalTileCount = tileSpan.x * tileSpan.y; // number of CAPAGlobalTile generated
		// env.value.tilePassGroups_Size32 = uvec4((min(env.value.pathTileCount, pc.maxPathTileCount) + 1023u) / 1024u, 1u, 1u, 0u);
		env.value.tilePassGroups_Size32 = uvec4((min(env.value.pathTileCount, pc.maxPathTileCount) + 31u) / 32u, 1u, 1u, 0u);
		env.value.orderPassGroups_Size32 = uvec4((env.value.globalTileCount + 31u) / 32u, 1u, 1u, 0u);
		env.value.binPassGroups_Size64 = uvec4((min(env.value.taskCount, pc.maxTaskCount) + 63u) / 64u, 1u, 1u, 0u);
		env.value.prefixPassGroups_Size16_2 = uvec4((min(env.value.pathTileRowCount, pc.maxPathTileRowCount) + 1u) / 2u, 1u, 1u, 0u);
		env.value.compositePassGroups_Size16_16 = uvec4(tileSpan.x, tileSpan.y, 1u, 0u);
	}
}
