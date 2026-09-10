#ifndef _VKGPGPU_H
#define _VKGPGPU_H 1
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#define GPU_MAX_DESCRIPTOR_SETS        1024
#define GPU_MAX_STORAGE_DESCRIPTORS    4096
#define GPU_MAX_UNIFORM_DESCRIPTORS    1024

#ifdef _WIN32
	#ifdef VKGPGPU_BUILD
		#define VKGPGPU_API __declspec(dllexport)
	#else
		#define VKGPGPU_API __declspec(dllimport)
	#endif
#else
	#define VKGPGPU_API
#endif

typedef enum gpu_mem_type {
	MEM_STAGING, /* Memory that is accessible from the CPU and GPU */
	MEM_GPU      /* Memory that is only available from the GPU */
} GpuMemType;
typedef struct vkalloc GpuAlloc;

typedef struct GpuContext GpuContext;
typedef struct GpuBuffer GpuBuffer;
typedef struct GpuBufferBinding GpuBufferBinding;
typedef struct GpuPushConstants GpuPushConstants;
typedef struct GpuPushConstantMember GpuPushConstantMember;
typedef struct GpuPushConstantLayout GpuPushConstantLayout;
typedef struct GpuProgram GpuProgram;
typedef struct GpuCommand GpuCommand;

//GpuContext
VKGPGPU_API GpuContext* gpu_ctx_init();
VKGPGPU_API void gpu_ctx_destroy(GpuContext* ctx);



//GpuBuffer
VKGPGPU_API GpuBuffer* gpu_buf_create(GpuContext *ctx, uint64_t size, GpuMemType type);
VKGPGPU_API void gpu_buf_destroy(GpuBuffer *buf);
VKGPGPU_API void gpu_buf_map(GpuBuffer *buf, void** data);
VKGPGPU_API void gpu_buf_unmap(GpuBuffer *buf);

//GpuProgram
VKGPGPU_API GpuProgram* gpu_program_create(GpuContext *ctx, void* shaderCode, uint64_t shaderSize);
VKGPGPU_API GpuProgram* gpu_program_load(GpuContext *ctx, const char* filename);
VKGPGPU_API void gpu_program_destroy(GpuProgram *program);

//GpuCommand
VKGPGPU_API GpuCommand* gpu_command_begin(GpuContext *ctx);

VKGPGPU_API void gpu_command_bind_buffer(GpuCommand *cmd, uint64_t id, GpuBuffer* buffer);
VKGPGPU_API void gpu_command_set_constant_ex(GpuCommand *cmd, GpuProgram *prog, char* name, void* data);
#define gpu_set(cmd, p, var) gpu_command_set_constant_ex(cmd, p, #var, &(var))
VKGPGPU_API void gpu_command_dispatch(GpuCommand *cmd, GpuProgram *program, uint32_t group_x, uint32_t group_y, uint32_t group_z);
VKGPGPU_API void gpu_command_submit(GpuCommand *cmd);
VKGPGPU_API void gpu_command_destroy(GpuCommand *cmd);


#endif
