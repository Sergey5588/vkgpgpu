#ifndef __VK_HELPER_H
#define __VK_HELPER_H 1

#include "../vendor/volk.h"
#include "stdio.h"
#include "stdlib.h"


const char* vk_result_to_str(VkResult result);

// Minimal assertion macro for your framework
#define VK_CHECK(f) \
    do { \
        VkResult res = (f); \
        if (res != VK_SUCCESS) { \
            printf("Fatal Vulkan Error at %s:%d: %s (%x)\n", __FILE__, __LINE__, vk_result_to_str(res), res); \
			abort(); \
        } \
    } while(0)
#endif
