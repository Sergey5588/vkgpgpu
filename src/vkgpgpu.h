#ifndef __VKGPGPU_H
#define __VKGPGPU_H
#include "../vendor/volk.h"

typedef struct {
	VkDevice device;
} GpuContext;

GpuContext gpu_init_context() {
	GpuContext ctx;


	return ctx;
}

#endif
