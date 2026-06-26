#include "../vendor/volk.h"
#include "../include/vkgpgpu.h"
#include "vk_helper.h"
#include "todo.h"

#include "../vendor/vkalloc.h"
GpuContext* gpu_ctx_init() {
	GpuContext* ctx = calloc(1, sizeof(GpuContext));
	//instance
	VK_CHECK(volkInitialize());
	VkApplicationInfo appInfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "vkgpgpu",
		.applicationVersion = VK_API_VERSION_1_3
	};
	VkInstanceCreateInfo instanceCI = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo

	};
	VK_CHECK(vkCreateInstance(&instanceCI, NULL, &ctx->instance));
	volkLoadInstance(ctx->instance);
	//device loading
	uint32_t physicalDeviceCount = 0;
	VK_CHECK(vkEnumeratePhysicalDevices(ctx->instance, &physicalDeviceCount,0));
	if(physicalDeviceCount == 0) {
		printf("No Vulkan devices found.");
		abort();
	}
	VkPhysicalDevice* const physicalDevices = malloc(sizeof(VkPhysicalDevice)*physicalDeviceCount);
	VK_CHECK(vkEnumeratePhysicalDevices(ctx->instance, &physicalDeviceCount,physicalDevices));
	
	for(size_t i= 0; i< physicalDeviceCount; i++) {
		VkPhysicalDeviceProperties2 deviceProperties = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
		VkPhysicalDevice pDevice = physicalDevices[i];
		vkGetPhysicalDeviceProperties2(pDevice, &deviceProperties);
		if(deviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			ctx->physicalDevice = pDevice;
			break;
		}
	}
	if(ctx->physicalDevice == VK_NULL_HANDLE) {
		ctx->physicalDevice = physicalDevices[0];
	}
	
	VkPhysicalDeviceProperties2 deviceProperties = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
	vkGetPhysicalDeviceProperties2(ctx->physicalDevice, &deviceProperties);
	printf("Running on: %s\n", deviceProperties.properties.deviceName);
	free(physicalDevices);
	//queue setup
	uint32_t queueFamilyPropertiesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(ctx->physicalDevice,&queueFamilyPropertiesCount, 0);
	VkQueueFamilyProperties* const queueFamilyProperties = malloc(sizeof(VkQueueFamilyProperties)*queueFamilyPropertiesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(ctx->physicalDevice,&queueFamilyPropertiesCount, queueFamilyProperties);
	ctx->computeFamilyIndex = UINT32_MAX;
	for(size_t i = 0; i<queueFamilyPropertiesCount; i++) {
		if(queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
			if(!(queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
				ctx->computeFamilyIndex = i;
				break;
			}
		}
	}
	if(ctx->computeFamilyIndex == UINT32_MAX) {
		for(size_t i = 0; i<queueFamilyPropertiesCount; i++) {
			if(queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
				ctx->computeFamilyIndex = i;
				break;
			}
		}
	}
	const float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo queueCI = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = ctx->computeFamilyIndex,
		.queueCount = 1,
		.pQueuePriorities = &queuePriority
	};
	VkDeviceCreateInfo deviceCI = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queueCI,
	};
	//create logical device
	VK_CHECK(vkCreateDevice(ctx->physicalDevice,&deviceCI,NULL,&ctx->device));
	volkLoadDevice(ctx->device);
	//create queue
	vkGetDeviceQueue(ctx->device,ctx->computeFamilyIndex,0, &ctx->queue);

	vkalloc_init(ctx->physicalDevice, ctx->device);
	free(queueFamilyProperties);
	VkCommandPoolCreateInfo cmdPoolCI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.queueFamilyIndex = ctx->computeFamilyIndex,
	};
	VK_CHECK(vkCreateCommandPool(ctx->device,&cmdPoolCI, NULL,&ctx->cmdPool));
	return ctx;
}

void gpu_ctx_destroy(GpuContext* ctx) {
	vkalloc_destroy();
	TODO("GPU Context cleanup");
	free(ctx);
}
