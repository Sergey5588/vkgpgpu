#ifndef _INTERNAL_H 
#define _INTERNAL_H 1
#include <vulkan/vulkan.h>
#include "../include/vkgpgpu.h"

struct GpuContext {
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue queue;
	uint32_t computeFamilyIndex;
	VkCommandPool cmdPool;
	VkDescriptorPool descriptorPool;
};

struct GpuBuffer {
	GpuContext *ctx;
	VkBuffer buffer;
	GpuAlloc *alloc;
	uint64_t size;
};
struct GpuBufferBinding {
	uint32_t binding;
	GpuBuffer *buffer;
};


struct GpuPushConstants {
    void *data;
    uint32_t size;
};

struct GpuPushConstantMember {
    char *name;
    uint32_t offset;
    uint32_t size;
};

struct GpuPushConstantLayout {
    uint32_t size;
    uint32_t memberCount;
    GpuPushConstantMember *members;
};

struct GpuProgram {
	GpuContext *ctx;
	GpuPushConstantLayout pcLayout;
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSetLayout descriptorSetLayout;
};

struct GpuCommand {
	GpuContext *ctx;
	VkCommandBuffer cmdBuffer;
	uint32_t bindingCount;
	GpuBufferBinding *bindings;
	GpuPushConstants *pushConstants;
};
#endif
