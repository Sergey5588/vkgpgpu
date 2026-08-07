#include "../vendor/volk.h"
#include "../include/vkgpgpu.h"
#include <stdio.h>
#include <stdlib.h>
#include "vk_helper.h"
#include <stdarg.h>
#include <string.h>
#include "todo.h"
#include "../vendor/spirv_reflect.h"
#include <assert.h>
char *gpu_strdup(const char *str){
    size_t len = strlen(str) + 1;
    char *copy = malloc(len);

    if (!copy)
        return NULL;

    memcpy(copy, str, len);

    return copy;
}
GpuProgram* gpu_program_create(GpuContext *ctx, const char* filename) {
	GpuProgram *program = calloc(1, sizeof(GpuProgram));
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
	VkShaderModule shaderModule;
	VK_CHECK(vkCreateShaderModule(ctx->device, &shaderCI, NULL, &shaderModule));
	SpvReflectShaderModule reflect;
	SpvReflectResult result = spvReflectCreateShaderModule(shaderSize, shaderCode, &reflect);
	assert(result == SPV_REFLECT_RESULT_SUCCESS);
	free(shaderCode);
	uint32_t bindingCount = 0;
	result = spvReflectEnumerateDescriptorBindings(&reflect, &bindingCount, NULL);
	assert(result == SPV_REFLECT_RESULT_SUCCESS);
	SpvReflectDescriptorBinding **reflectBindings = malloc(bindingCount * sizeof(SpvReflectDescriptorBinding*));
	result = spvReflectEnumerateDescriptorBindings(&reflect, &bindingCount, reflectBindings);
	assert(result == SPV_REFLECT_RESULT_SUCCESS);
	VkDescriptorSetLayoutBinding bindings[bindingCount];
	TODO("Remove VLA");
	for(uint32_t i=0; i < bindingCount; i++) {
		SpvReflectDescriptorBinding* refl = reflectBindings[i];
		bindings[i] = (VkDescriptorSetLayoutBinding){
			.binding = refl->binding,
			.descriptorType = (VkDescriptorType)refl->descriptor_type,
			.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
		};
		uint32_t descCount = 1;
		if (refl->type_description && refl->type_description->traits.array.dims_count > 0) {
			descCount = refl->type_description->traits.array.dims[0];
		}
		bindings[i].descriptorCount = descCount;
	}
	free(reflectBindings);
	VkDescriptorSetLayoutCreateInfo dsLayoutCI = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = bindingCount,
		.pBindings = bindings,
	};
	VK_CHECK(vkCreateDescriptorSetLayout(ctx->device, &dsLayoutCI, NULL, &program->descriptorSetLayout));

	uint32_t pcCount = 0;
	result = spvReflectEnumeratePushConstantBlocks(&reflect, &pcCount, NULL);
	assert(result == SPV_REFLECT_RESULT_SUCCESS);
	assert(pcCount <= 1);
	VkPushConstantRange pushConstantRange;
	if(pcCount == 1) {
		SpvReflectBlockVariable **reflectPushConstants= malloc(pcCount * sizeof(SpvReflectBlockVariable*));
		result = spvReflectEnumeratePushConstantBlocks(&reflect, &pcCount, reflectPushConstants);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);
		pushConstantRange = (VkPushConstantRange){
			.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
			.offset = reflectPushConstants[0]->offset,
			.size =reflectPushConstants[0]->size
		};
		program->pcLayout.memberCount = reflectPushConstants[0]->member_count;
		program->pcLayout.members = malloc(program->pcLayout.memberCount*sizeof(GpuPushConstantMember));
		program->pcLayout.size = reflectPushConstants[0]->size;
		for(size_t i = 0; i< program->pcLayout.memberCount; ++i) {
			program->pcLayout.members[i] = (GpuPushConstantMember){
				.size = reflectPushConstants[0]->members[i].size,
				.name = gpu_strdup(reflectPushConstants[0]->members[i].name),
				.offset = reflectPushConstants[0]->members[i].offset,
			};
		}
		free(reflectPushConstants);
	}
	VkPipelineLayoutCreateInfo pipelineLayoutCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1,
		.pSetLayouts = &program->descriptorSetLayout,
		.pushConstantRangeCount = pcCount,
		.pPushConstantRanges = pcCount ? &pushConstantRange : NULL,
	};
	VkPipelineLayout pipelineLayout;
	VK_CHECK(vkCreatePipelineLayout(ctx->device, &pipelineLayoutCI, NULL, &pipelineLayout));
	VkComputePipelineCreateInfo pipelineCI = {
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.stage = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_COMPUTE_BIT,
			.module = shaderModule,
			.pName = "main",
		},
		.layout = pipelineLayout
	};
	VkPipeline pipeline;
	VK_CHECK(vkCreateComputePipelines(ctx->device,VK_NULL_HANDLE, 1, &pipelineCI, NULL, &pipeline));

	program->ctx = ctx;
	program->pipeline = pipeline;
	program->pipelineLayout = pipelineLayout;
	vkDestroyShaderModule(ctx->device, shaderModule, NULL);
	spvReflectDestroyShaderModule(&reflect);
	return program;
}
void gpu_program_destroy(GpuProgram *program) {
	GpuContext *ctx = program->ctx;
	for (uint32_t i =0; i < program->pcLayout.memberCount; ++i) {
		free(program->pcLayout.members[i].name);
	}
	free(program->pcLayout.members);
	vkDestroyDescriptorSetLayout(ctx->device,program->descriptorSetLayout, NULL);
	//vkDestroyDescriptorPool(ctx->device, program->descriptorPool, NULL);
	vkDestroyPipelineLayout(ctx->device, program->pipelineLayout, NULL);
	vkDestroyPipeline(ctx->device, program->pipeline, NULL);
	free(program);
}
