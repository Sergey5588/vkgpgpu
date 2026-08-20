#include "vk_helper.h"
#include "../include/vkgpgpu.h"
#include <stdlib.h>
#include "todo.h"
#include "string.h"
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
	cmd->ctx = ctx;
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
	if(cmd->bindingCount > 0) {

		VkDescriptorBufferInfo bufferInfos[cmd->bindingCount];
		for(uint32_t i = 0; i < cmd->bindingCount; i++) {
			bufferInfos[i] = (VkDescriptorBufferInfo){
				.buffer = cmd->bindings[i].buffer->buffer,
				.offset = 0,
				.range = VK_WHOLE_SIZE
							
			};
		}
		VkWriteDescriptorSet writes[cmd->bindingCount];
		for(uint32_t i = 0; i < cmd->bindingCount; i++) {
			writes[i] = (VkWriteDescriptorSet){
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = descriptorSet,
				.dstBinding = cmd->bindings[i].binding,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				.pBufferInfo = &bufferInfos[i]
			};
		}
		TODO("Different descriptor types");
		vkUpdateDescriptorSets(ctx->device,cmd->bindingCount, writes,0,NULL);
	}
	vkCmdBindPipeline(cmd->cmdBuffer,VK_PIPELINE_BIND_POINT_COMPUTE,program->pipeline);
	vkCmdBindDescriptorSets(cmd->cmdBuffer,VK_PIPELINE_BIND_POINT_COMPUTE,program->pipelineLayout, 0,1, &descriptorSet, 0, NULL);
	vkCmdPushConstants(cmd->cmdBuffer, program->pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, cmd->pushConstants->size, cmd->pushConstants->data);
	vkCmdDispatch(cmd->cmdBuffer, group_x, group_y, group_z);
}
void gpu_command_set_constant_ex(GpuCommand *cmd, GpuProgram *prog, char* name, void* data) {

	if(!cmd->pushConstants) {
		cmd->pushConstants = calloc(1, sizeof(GpuPushConstants));
		cmd->pushConstants->data = malloc(prog->pcLayout.size);
		cmd->pushConstants->size = prog->pcLayout.size;
	}
	GpuPushConstantMember *member;
	bool found = 0;
	for(size_t i = 0; i < prog->pcLayout.memberCount; ++i) {
		if(strcmp(prog->pcLayout.members[i].name, name) == 0) {
			member = &prog->pcLayout.members[i];
			found = 1;
		}
	}
	if(!found) {
		fprintf(stderr, "Field not found: %s\n", name);
		return;
	}
	memcpy(cmd->pushConstants->data + member->offset, data,member->size);
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
	VK_CHECK(vkResetDescriptorPool(ctx->device, ctx->descriptorPool,0));
}
void gpu_command_destroy(GpuCommand *cmd) {
	vkFreeCommandBuffers(cmd->ctx->device, cmd->ctx->cmdPool, 1,&cmd->cmdBuffer);
	if(cmd->bindings) free(cmd->bindings);
	if(cmd->pushConstants) {
		if(cmd->pushConstants->data) free(cmd->pushConstants->data);
		free(cmd->pushConstants);
	}
	if(cmd) free(cmd);
}
