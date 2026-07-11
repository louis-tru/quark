// @private head
#ifndef __vk_shader_natives_1783794452675
#define __vk_shader_natives_1783794452675
#include <cstdint>
namespace qk {
	struct VKShaderCode {
		const uint32_t *words;
		uint32_t size;
	};
	struct VKShaderSource {
		const char *name;
		VKShaderCode vertex;
		VKShaderCode fragment;
		VKShaderCode compute;
	};
	extern const VKShaderSource vkShaderSources[];
	extern const uint32_t vkShaderSourceCount;
}
#endif
