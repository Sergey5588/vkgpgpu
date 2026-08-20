#ifndef __VKGPGPU_H
#define __VKGPGPU_H 1
#include <vulkan/vulkan.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#define GPU_MAX_DESCRIPTOR_SETS        1024
#define GPU_MAX_STORAGE_DESCRIPTORS    4096
#define GPU_MAX_UNIFORM_DESCRIPTORS    1024


typedef enum gpu_mem_type {
	MEM_STAGING, /* Memory that is accessible from the CPU and GPU */
	MEM_GPU      /* Memory that is only available from the GPU */
} GpuMemType;
typedef struct vkalloc GpuAlloc;

typedef struct {
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue queue;
	uint32_t computeFamilyIndex;
	VkCommandPool cmdPool;
	VkDescriptorPool descriptorPool;
} GpuContext;

typedef struct {
	GpuContext *ctx;
	VkBuffer buffer;
	GpuAlloc *alloc;
	uint64_t size;
} GpuBuffer;
typedef struct {
	uint32_t binding;
	GpuBuffer *buffer;
} GpuBufferBinding;


typedef struct {
    void *data;
    uint32_t size;
} GpuPushConstants;

typedef struct {
    char *name;
    uint32_t offset;
    uint32_t size;
} GpuPushConstantMember;

typedef struct {
    uint32_t size;
    uint32_t memberCount;
    GpuPushConstantMember *members;
} GpuPushConstantLayout;

typedef struct {
	GpuContext *ctx;
	GpuPushConstantLayout pcLayout;
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSetLayout descriptorSetLayout;
} GpuProgram;

typedef struct {
	GpuContext *ctx;
	VkCommandBuffer cmdBuffer;
	uint32_t bindingCount;
	GpuBufferBinding *bindings;
	GpuPushConstants *pushConstants;
} GpuCommand;
//GpuContext
GpuContext* gpu_ctx_init();
void gpu_ctx_destroy(GpuContext* ctx);



//GpuBuffer
GpuBuffer* gpu_buf_create(GpuContext *ctx, uint64_t size, GpuMemType type);
void gpu_buf_destroy(GpuBuffer *buf);
void gpu_buf_map(GpuBuffer *buf, void** data);
void gpu_buf_unmap(GpuBuffer *buf);

//GpuProgram
GpuProgram* gpu_program_create(GpuContext *ctx, void* shaderCode, uint64_t shaderSize);
GpuProgram* gpu_program_load(GpuContext *ctx, const char* filename);
void gpu_program_destroy(GpuProgram *program);

//GpuCommand
GpuCommand* gpu_command_begin(GpuContext *ctx);

void gpu_command_bind_buffer(GpuCommand *cmd, uint64_t id, GpuBuffer* buffer);
void gpu_command_set_constant_ex(GpuCommand *cmd, GpuProgram *prog, char* name, void* data);
#define gpu_set(cmd, p, var) gpu_command_set_constant_ex(cmd, p, #var, &(var))
void gpu_command_dispatch(GpuCommand *cmd, GpuProgram *program, uint32_t group_x, uint32_t group_y, uint32_t group_z);

void gpu_command_submit(GpuCommand *cmd);
void gpu_command_destroy(GpuCommand *cmd);


#endif
