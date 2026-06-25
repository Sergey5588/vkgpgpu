#include "../vendor/volk.h"
#include "../include/vkgpgpu.h"
#include <stdio.h>
#include <stdlib.h>
#include "vk_helper.h"
GpuKernel* gpu_kernel_create(GpuContext *ctx, const char* filename) {
	GpuKernel *kernel = calloc(1, sizeof(GpuKernel));
	FILE *f = fopen(filename, "rb");
	if(f == NULL) {
		fprintf(stderr, "Failed to open file: %s", filename);
		return NULL;
	}
	fseek(f, 0, SEEK_END);
	size_t shaderSize = ftell(f);
	uint32_t *shaderCode = malloc(shaderSize);
	fread(shaderCode, 1,shaderSize,f);
	fclose(f);
	VkShaderModuleCreateInfo shaderCI = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = shaderSize, 
		.pCode = shaderCode,
	};
	VK_CHECK(vkCreateShaderModule(ctx->device, &shaderCI, NULL, &kernel->shaderModule));
	free(shaderCode);
	return kernel;
}
