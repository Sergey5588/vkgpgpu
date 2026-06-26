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
	VkCommandPool cmdPool;
} GpuContext;

typedef struct {
	GpuContext *ctx;
	VkBuffer buffer;
	GpuAlloc *alloc;
	uint64_t size;
} GpuBuffer;

typedef struct {
	GpuContext *ctx;
	VkShaderModule shaderModule;
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSet;
	uint32_t bufferCount;
	GpuBuffer **buffers;
} GpuKernel;
GpuContext* gpu_ctx_init();
void gpu_ctx_destroy(GpuContext* ctx);




GpuBuffer* gpu_buf_create(GpuContext *ctx, uint64_t size, uint8_t type);
void gpu_buf_destroy(GpuBuffer *buf);
void gpu_buf_map(GpuBuffer *buf, void** data);
void gpu_buf_unmap(GpuBuffer *buf);


GpuKernel* gpu_kernel_create(GpuContext *ctx, const char* filename, uint32_t bufferCount, ...);
void gpu_kernel_dispatch(GpuKernel *kernel, uint32_t group_x, uint32_t group_y, uint32_t group_z);
void gpu_kernel_destroy(GpuKernel *kernel);
#endif
