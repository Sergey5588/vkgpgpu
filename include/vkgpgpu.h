#ifndef __VKGPGPU_H
#define __VKGPGPU_H 1
#include "../vendor/volk.h"

typedef struct {
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue queue;
	uint32_t computeFamilyIndex;
} GpuContext;

GpuContext gpu_ctx_init();
void gpu_ctx_destroy(GpuContext* ctx);
#endif
