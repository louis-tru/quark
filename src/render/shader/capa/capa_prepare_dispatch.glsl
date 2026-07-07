// CAPA prepare dispatch pass.
// Publish global tile span/count and indirect dispatch group counts for the
// GPU-driven CAPA passes.

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
		env.value.realTaskCount = min(env.value.taskCount, pc.maxTaskCount);
		env.value.realPathTileCount = min(env.value.pathTileCount, pc.maxPathTileCount);
		env.value.realPathTileRowCount = min(env.value.pathTileRowCount, pc.maxPathTileRowCount);
		// compute the number of dispatch groups
		env.value.tilePassGroups_Size32 = uvec4((env.value.realPathTileCount + 31u) / 32u, 1u, 1u, 0u);
		env.value.layerPlanPassGroups_Size32 = uvec4((env.value.globalTileCount + 31u) / 32u, 1u, 1u, 0u);
		env.value.binPassGroups_Size32 = uvec4((env.value.realTaskCount + 31u) / 32u, 1u, 1u, 0u);
		env.value.classifyPassGroups_Size32 = uvec4((env.value.realPathTileRowCount + 31u) / 32u, 1u, 1u, 0u);
		env.value.prefixPassGroups_Size16_2 = uvec4((env.value.realPathTileRowCount + 1u) / 2u, 1u, 1u, 0u);
		// coveragePassGroups_Size16_2.x and .w are finalized by layer_plan after
		// it knows how many boundary tiles survived ordering/culling.
		env.value.coveragePassGroups_Size16_2 = uvec4(0u, 1u, 1u, 0u);
		env.value.compositePassGroups_Size16_16 = uvec4(uint(tileSpan.x) * 2u, uint(tileSpan.y) * 2u, 1u, 0u);
	}
}
