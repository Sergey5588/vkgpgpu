#ifndef __VKGPGPU_H
#define __VKGPGPU_H 1
#include <vulkan/vulkan.h>
typedef struct vkalloc GpuAlloc;
typedef struct {
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue queue;
	uint32_t computeFamilyIndex;
} GpuContext;

typedef struct {
	GpuContext *ctx;
	VkBuffer buffer;
	GpuAlloc *alloc;
	uint64_t size;
} GpuBuffer;

GpuContext* gpu_ctx_init();
void gpu_ctx_destroy(GpuContext* ctx);




GpuBuffer* gpu_buf_create(GpuContext *ctx, uint64_t size, uint8_t type);
void gpu_buf_destroy(GpuBuffer *buf);
void gpu_buf_map(GpuBuffer *buf, void** data);
void gpu_buf_unmap(GpuBuffer *buf);
#endif
