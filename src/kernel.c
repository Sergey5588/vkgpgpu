#include "../vendor/volk.h"
#include "../include/vkgpgpu.h"
#include <stdio.h>
#include <stdlib.h>
#include "vk_helper.h"
#include <stdarg.h>
#include "todo.h"
GpuKernel* gpu_kernel_create(GpuContext *ctx, const char* filename, uint32_t bufferCount, ...) {
	GpuKernel *kernel = calloc(1, sizeof(GpuKernel));
	FILE *f = fopen(filename, "rb");
	if(f == NULL) {
		fprintf(stderr, "Failed to open file: %s\n", filename);
		return NULL;
	}
	fseek(f, 0, SEEK_END);
	size_t shaderSize = ftell(f);
	if(shaderSize <= 0 || shaderSize%4 !=0) {
		fprintf(stderr, "Bad SPIR-V size.\n");
		fclose(f);
		return NULL;
	}
	uint32_t *shaderCode = malloc(shaderSize);
	rewind(f);
	fread(shaderCode, 1,shaderSize,f);
	printf("First SPIR-V dword: 0x%08x\n", shaderCode[0]);
	fclose(f);
	VkShaderModuleCreateInfo shaderCI = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = shaderSize, 
		.pCode = shaderCode,
	};
	VK_CHECK(vkCreateShaderModule(ctx->device, &shaderCI, NULL, &kernel->shaderModule));
	free(shaderCode);
	va_list args;
	va_start(args,bufferCount);
	VkDescriptorSetLayoutBinding bindings[bufferCount];
	for(uint32_t i=0; i < bufferCount; i++) {
		bindings[i] = (VkDescriptorSetLayoutBinding){
			.binding = i,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
		};
	}
	VkDescriptorSetLayoutCreateInfo dsLayoutCI = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = bufferCount,
		.pBindings = bindings,
	};
	VkDescriptorSetLayout dsLayout;
	VK_CHECK(vkCreateDescriptorSetLayout(ctx->device, &dsLayoutCI, NULL, &dsLayout));

	VkPipelineLayoutCreateInfo pipelineLayoutCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1,
		.pSetLayouts = &dsLayout
	};
	VkPipelineLayout pipelineLayout;
	VK_CHECK(vkCreatePipelineLayout(ctx->device, &pipelineLayoutCI, NULL, &pipelineLayout));

	VkComputePipelineCreateInfo pipelineCI = {
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.stage = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_COMPUTE_BIT,
			.module = kernel->shaderModule,
			.pName = "main",
		},
		.layout = pipelineLayout
	};
	VkPipeline pipeline;
	VK_CHECK(vkCreateComputePipelines(ctx->device,VK_NULL_HANDLE, 1, &pipelineCI, NULL, &pipeline));
	VkDescriptorPoolSize poolSize = {
		.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = bufferCount,
	};
	VkDescriptorPoolCreateInfo poolCI = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = 1,
		.poolSizeCount = 1,
		.pPoolSizes = &poolSize,
	};
	VkDescriptorPool descriptorPool;
	VK_CHECK(vkCreateDescriptorPool(ctx->device, &poolCI, NULL, &descriptorPool));
	VkDescriptorSetAllocateInfo allocSetCI = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &dsLayout
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
	va_end(args);
	VkWriteDescriptorSet writes[bufferCount];
	for(uint32_t i = 0; i < bufferCount; i++) {
		writes[i] = (VkWriteDescriptorSet){
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = descriptorSet,
			.dstBinding = i,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.pBufferInfo = &bufferInfos[i]
		};
	}
	vkUpdateDescriptorSets(ctx->device,bufferCount, writes,0,NULL);
	va_start(args,bufferCount);
	kernel->ctx = ctx;
	kernel->pipeline = pipeline;
	kernel->pipelineLayout = pipelineLayout;
	kernel->descriptorSetLayout = dsLayout;
	kernel->descriptorPool = descriptorPool;
	kernel->descriptorSet = descriptorSet;
	kernel->bufferCount = bufferCount;
	kernel->buffers = calloc(bufferCount, sizeof(GpuBuffer*));
	for(uint32_t i = 0; i < bufferCount; i++) {
		kernel->buffers[i] = va_arg(args,GpuBuffer*);
	}
	return kernel;
}
void gpu_kernel_dispatch(GpuKernel *kernel, uint32_t group_x, uint32_t group_y, uint32_t group_z) {
	GpuContext *ctx = kernel->ctx;
	VkCommandBuffer cmdBuffer;
	VkCommandBufferAllocateInfo cmdBufAllocCI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool =ctx->cmdPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};
	VK_CHECK(vkAllocateCommandBuffers(ctx->device, &cmdBufAllocCI, &cmdBuffer));
	VkCommandBufferBeginInfo beginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
	};
	VK_CHECK(vkBeginCommandBuffer(cmdBuffer, &beginInfo));
	vkCmdBindPipeline(cmdBuffer,VK_PIPELINE_BIND_POINT_COMPUTE, kernel->pipeline);
	vkCmdBindDescriptorSets(cmdBuffer,VK_PIPELINE_BIND_POINT_COMPUTE, kernel->pipelineLayout, 0,1,&kernel->descriptorSet,0,NULL);
	vkCmdDispatch(cmdBuffer, group_x, group_y, group_z);
	vkEndCommandBuffer(cmdBuffer);
	VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &cmdBuffer
	};
	VK_CHECK(vkQueueSubmit(ctx->queue, 1, &submitInfo,VK_NULL_HANDLE));
	vkQueueWaitIdle(ctx->queue);
	vkFreeCommandBuffers(ctx->device, ctx->cmdPool,1,&cmdBuffer);
}
void gpu_kernel_destroy(GpuKernel *kernel) {
	TODO("Kernel destroy");
}
