/*
zlib License

(C) 2022 Leon Marz <main@lmarz.org>

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/


/* Modified version:
   - Added '#include "volk.h"' in vkalloc.c
   Original file copyright: Leon Marz <main@lmarz.org>
*/

#ifndef VKALLOC_H
#define VKALLOC_H

#include <vulkan/vulkan.h>


enum mem_type {
	MEM_TYPE_STAGING, /* Memory that is accessible from the CPU and GPU */
	MEM_TYPE_GPU /* Memory that is only available from the GPU */
};

struct vkalloc {
	uint32_t mem_type; /* The memory type */
	VkDeviceMemory memory; /* The memory handle */
	uint32_t offset; /* The offset in the memory block */
	size_t size; /* The size of the memory block. Bigger than requested size */
};

/**
 * Initialize the vkalloc module. Needs to be called before any other call in
 * this module. Must be recalled, if the device changes.
 * 
 * @physical_device: The physical device this module should run on
 * @dev: The logical device based on the physical device
 */
void vkalloc_init(VkPhysicalDevice physical_device, VkDevice dev);

/**
 * Create a buffer and allocate memory for it.
 * 
 * @buffer_info: The buffer create info struct
 * @type: The memory type the buffer should be using
 * @buffer: A pointer to the buffer handle. Output
 * @alloc: A pointer to the memory information struct. Output
 * 
 * @returns VK_SUCCESS for success. Anything else for error
 */
VkResult vkalloc_create_buffer(VkBufferCreateInfo *buffer_info, enum mem_type type, VkBuffer *buffer, struct vkalloc **alloc);

/**
 * Create an image and allocate memory for it.
 * 
 * @image_info: The image create info struct
 * @type: The memory type the image should be using
 * @image: A pointer to the image handle. Output
 * @alloc: A pointer to the memory information struct. Output
 * 
 * @returns VK_SUCCESS for success. Anything else for error
 */
VkResult vkalloc_create_image(VkImageCreateInfo *image_info, enum mem_type type, VkImage *image, struct vkalloc **alloc);

/**
 * Map staging type memory.
 * 
 * @vkalloc: The part of memory that should be mapped to CPU memory
 * @data: A pointer that will point to the CPU mapped memory.
 * 
 * @returns VK_SUCCESS for success. Anything else for error
 */
VkResult vkalloc_map(struct vkalloc *vkalloc, void **data);

/**
 * Unmap mapped memory. WARNING: this will unmap everything that uses this
 * VkDeviceMemory.
 * 
 * @vkalloc: The memory information that has been previously mapped.
 */
void vkalloc_unmap(struct vkalloc *vkalloc);

/**
 * Free memory after the buffer or image has been destroyed.
 * 
 * @vkalloc: The memory information that should be freed
 * 
 * @returns VK_SUCCESS for success. VK_ERROR_OUT_OF_HOST_MEMORY for when no more
 * ram to save the information is available.
 */
VkResult vkalloc_free(struct vkalloc *vkalloc);

/**
 * Destroy everything this module has allocated on CPU and GPU memory.
 */
void vkalloc_destroy(void);

#endif /* VKALLOC_H */
