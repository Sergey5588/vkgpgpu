#include "vk_helper.h"
#include "../include/vkgpgpu.h"
#include <stdlib.h>
#include "todo.h"
GpuCommand* gpu_command_begin(GpuContext *ctx) {
	GpuCommand* cmd = calloc(1, sizeof(GpuCommand));
	VkCommandBufferAllocateInfo cmdBufAllocCI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool =ctx->cmdPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};
	VK_CHECK(vkAllocateCommandBuffers(ctx->device, &cmdBufAllocCI, &cmd->cmdBuffer));
	VkCommandBufferBeginInfo beginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
	};
	VK_CHECK(vkBeginCommandBuffer(cmd->cmdBuffer, &beginInfo));

	return cmd;
}
void gpu_command_bind_buffer(GpuCommand *cmd, uint64_t binding, GpuBuffer* buffer) {
	if(!cmd->bindings) {
		cmd->bindings = malloc(sizeof(GpuBufferBinding));

	} else {
		cmd->bindings = realloc(cmd->bindings, (cmd->bindingCount+1)*(sizeof(GpuBufferBinding)));
	}
	cmd->bindings[cmd->bindingCount] = (GpuBufferBinding){binding,buffer};
	cmd->bindingCount++;
}
void gpu_command_dispatch(GpuCommand *cmd, GpuProgram *program, uint32_t group_x, uint32_t group_y, uint32_t group_z) {
	GpuContext *ctx = cmd->ctx;
	VkDescriptorSetAllocateInfo allocSetCI = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = ctx->descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &program->descriptorSetLayout
	};
	VkDescriptorSet descriptorSet;
	VK_CHECK(vkAllocateDescriptorSets(ctx->device, &allocSetCI, &descriptorSet));
	VkDescriptorBufferInfo bufferInfos[bufferCount];
	for(uint32_t i = 0; i < bufferCount; i++) {
		bufferInfos[i] = (VkDescriptorBufferInfo){
			.buffer = va_arg(args, GpuBuffer*)->buffer,
			.offset = 0,
			.range = VK_WHOLE_SIZE
						
		};
	}
	vkCmdBindPipeline(cmd->cmdBuffer,VK_PIPELINE_BIND_POINT_COMPUTE,program->pipeline);
	vkCmdBindDescriptorSets(cmd->cmdBuffer,VK_PIPELINE_BIND_POINT_COMPUTE,program->pipelineLayout, 0,1, ctx->descriptorPool)
	vkCmdDispatch(cmd->cmdBuffer, group_x, group_y, group_z);
}

void gpu_command_submit(GpuCommand *cmd) {

	GpuContext *ctx = cmd->ctx;
	vkEndCommandBuffer(cmd->cmdBuffer);
	VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &cmd->cmdBuffer
	};
	VK_CHECK(vkQueueSubmit(ctx->queue, 1, &submitInfo,VK_NULL_HANDLE));
	vkQueueWaitIdle(ctx->queue);
}
void gpu_command_destroy(GpuCommand *cmd) {
	TODO("GpuCommand destruction");
}
