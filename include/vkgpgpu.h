#ifndef __VKGPGPU_H
#define __VKGPGPU_H 1
#include <vulkan/vulkan.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
typedef struct vkalloc GpuAlloc;
typedef struct {
	void* data;
	size_t size;
} GpuConsts ;
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
	GpuConsts consts;
} GpuKernel;
GpuContext* gpu_ctx_init();
void gpu_ctx_destroy(GpuContext* ctx);




GpuBuffer* gpu_buf_create(GpuContext *ctx, uint64_t size, uint8_t type);
void gpu_buf_destroy(GpuBuffer *buf);
void gpu_buf_map(GpuBuffer *buf, void** data);
void gpu_buf_unmap(GpuBuffer *buf);


GpuKernel* gpu_kernel_create(GpuContext *ctx, const char* filename, GpuConsts consts, uint32_t bufferCount, ...);
void gpu_kernel_dispatch(GpuKernel *kernel, uint32_t group_x, uint32_t group_y, uint32_t group_z);
void gpu_kernel_destroy(GpuKernel *kernel);

void gpu_const_push_ex(GpuConsts *consts, void* data, size_t size, size_t alignment);

#define GPU_STD430_ALIGN_OF(type) _Generic((type), \
	bool: 4, \
	char: 1, \
	int8_t: 1, \
	uint8_t: 1, \
	float: 4, \
	int32_t: 4, \
	uint32_t: 4, \
	double: 8, \
	int64_t: 8, \
	uint64_t: 8, \
	default: sizeof(type) \
)
#define GPU_PTR_OF(val) _Generic((val), \
	bool: &(bool){val}, \
	char: &(char){val}, \
	int8_t: &(int8_t){val}, \
	uint8_t: &(uint8_t){val}, \
    float:    &(float){val}, \
    int32_t:  &(int32_t){val}, \
    uint32_t: &(uint32_t){val}, \
    double:   &(double){val}, \
    int64_t:  &(int64_t){val}, \
    uint64_t: &(uint64_t){val} \
)
#define GPU_CONST_PUSH(consts, var) \
		gpu_const_push_ex((consts), GPU_PTR_OF(var),sizeof(var), GPU_STD430_ALIGN_OF(var))
#endif
