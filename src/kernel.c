#include "../vendor/volk.h"
#include "../include/vkgpgpu.h"
#include <stdio.h>
#include <stdlib.h>
#include "vk_helper.h"
#include <stdarg.h>
GpuKernel* gpu_kernel_create(GpuContext *ctx, const char* filename, uint32_t bufferCount, ...) {
	GpuKernel *kernel = calloc(1, sizeof(GpuKernel));
	FILE *f = fopen(filename, "rb");
	if(f == NULL) {
		fprintf(stderr, "Failed to open file: %s", filename);
		return NULL;
	}
	fseek(f, 0, SEEK_END);
	size_t shaderSize = ftell(f);
	uint32_t *shaderCode = malloc(shaderSize);
	fread(shaderCode, 1,shaderSize,f);
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
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
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
	return kernel;
}
