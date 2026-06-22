#include "../include/vkgpgpu.h"
#include "vk_helper.h"
#include "todo.h"
GpuContext gpu_ctx_init() {
	GpuContext ctx = {0};
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
	VK_CHECK(vkCreateInstance(&instanceCI, NULL, &ctx.instance));
	volkLoadInstance(ctx.instance);
	
	uint32_t physicalDeviceCount = 0;
	VK_CHECK(vkEnumeratePhysicalDevices(ctx.instance, &physicalDeviceCount,0));
	if(physicalDeviceCount == 0) {
		printf("No Vulkan devices found.");
		abort();
	}
	VkPhysicalDevice* const physicalDevices = malloc(sizeof(VkPhysicalDevice)*physicalDeviceCount);
	VK_CHECK(vkEnumeratePhysicalDevices(ctx.instance, &physicalDeviceCount,physicalDevices));
	
	for(uint32_t i= 0; i< physicalDeviceCount; i++) {
		VkPhysicalDeviceProperties2 deviceProperties = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
		VkPhysicalDevice pDevice = physicalDevices[i];
		vkGetPhysicalDeviceProperties2(pDevice, &deviceProperties);
		if(deviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			ctx.physicalDevice = pDevice;
			break;
		}
	}
	if(ctx.physicalDevice == VK_NULL_HANDLE) {
		ctx.physicalDevice = physicalDevices[0];
	}
	
	VkPhysicalDeviceProperties2 deviceProperties = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
	vkGetPhysicalDeviceProperties2(ctx.physicalDevice, &deviceProperties);
	printf("Running on: %s\n", deviceProperties.properties.deviceName);
	free(physicalDevices);
	return ctx;
}

void gpu_ctx_destroy(GpuContext* ctx) {
	TODO("GPU Context cleanup");
}
