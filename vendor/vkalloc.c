#include "vkalloc.h"

#include <stdlib.h>

#define MAX_POOLS 256
#define MEMORY_BLOCK 1024 * 1024


struct mem_node {
	struct vkalloc *alloc;
	struct mem_node *next;
};


static struct mem_node *head;
static struct mem_node *tail;

static VkDeviceMemory pools[MAX_POOLS];
static uint32_t pool_size = 0;

static VkPhysicalDeviceLimits limits;
static VkPhysicalDeviceMemoryProperties props;
static VkDevice device;


static uint32_t get_memory_type(uint32_t type_bits, VkMemoryPropertyFlags mem_props) {
	uint32_t i;

	for(i = 0; i < props.memoryTypeCount; i++) {
		if((type_bits & 1) == 1) {
			if((props.memoryTypes[i].propertyFlags & mem_props) == mem_props) {
				return i;
			}
		}
		type_bits >>= 1;
	}
	return UINT32_MAX;
}

static VkResult mem_insert(struct vkalloc *alloc) {
	struct mem_node *node;

	node = malloc(sizeof(struct mem_node));
	if(!node)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	node->alloc = alloc;
	node->next = NULL;

	if(!head)
		head = node;
	if(tail)
		tail->next = node;
	tail = node;

	return VK_SUCCESS;
}

static VkResult vkalloc(VkMemoryRequirements req, enum mem_type type, struct vkalloc **alloc) {
	VkMemoryPropertyFlags mem_type;
	VkResult res;
	struct mem_node *node;
	struct mem_node *last;

	switch(type) {
		case MEM_TYPE_STAGING:
			mem_type = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			break;
		case MEM_TYPE_GPU:
			mem_type = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			break;
		default:
			return VK_ERROR_UNKNOWN;
			break;
	}

	(*alloc) = malloc(sizeof(struct vkalloc));
	if(!(*alloc)) {
		res = VK_ERROR_OUT_OF_HOST_MEMORY;
		goto err_return;
	}

	(*alloc)->memory = VK_NULL_HANDLE;
	(*alloc)->size = req.size;
	(*alloc)->size += req.size % limits.bufferImageGranularity;
	(*alloc)->mem_type = get_memory_type(req.memoryTypeBits, mem_type);
	if((*alloc)->mem_type == UINT32_MAX) {
		res = VK_ERROR_UNKNOWN;
		goto err_alloc;
	}

	node = head;
	last = NULL;
	while(node) {
		if(node->alloc->mem_type == (*alloc)->mem_type && node->alloc->size >= (*alloc)->size) {
			(*alloc)->memory = node->alloc->memory;
			(*alloc)->offset = node->alloc->offset;

			if(node->alloc->size == (*alloc)->size) {
				if(last) {
					last->next = node->next;
				} else {
					head = node->next;
				}
				free(node->alloc);
				free(node);
			} else {
				node->alloc->offset += (*alloc)->size;
				node->alloc->size -= (*alloc)->size;
			}
			break;
		}
		last = node;
		node = node->next;
	}

	if(!(*alloc)->memory) {
		uint32_t count;
		VkMemoryAllocateInfo alloc_info;
		struct vkalloc *vkalloc;

		for(count = 1; count * MEMORY_BLOCK < (*alloc)->size; count++);

		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.pNext = NULL;
		alloc_info.allocationSize = count * MEMORY_BLOCK;
		alloc_info.memoryTypeIndex = (*alloc)->mem_type;

		(*alloc)->offset = 0;

		res = vkAllocateMemory(device, &alloc_info, NULL, &(*alloc)->memory);
		if(res != VK_SUCCESS)
			goto err_alloc;

		vkalloc = malloc(sizeof(struct vkalloc));
		if(!vkalloc) {
			res = VK_ERROR_OUT_OF_HOST_MEMORY;
			goto err_mem;
		}

		vkalloc->mem_type = (*alloc)->mem_type;
		vkalloc->memory = (*alloc)->memory;
		vkalloc->offset = (*alloc)->size;
		vkalloc->size = count * MEMORY_BLOCK - (*alloc)->size;

		res = mem_insert(vkalloc);
		if(res != VK_SUCCESS) {
			free(vkalloc);
			goto err_mem;
		}

		pools[pool_size] = (*alloc)->memory;
		pool_size++;
	}

	return VK_SUCCESS;

err_mem:
	vkFreeMemory(device, (*alloc)->memory, NULL);
err_alloc:
	free(*alloc);
err_return:
	return res;
}


void vkalloc_init(VkPhysicalDevice physical_device, VkDevice dev) {
	VkPhysicalDeviceProperties gpu_props;

	vkGetPhysicalDeviceProperties(physical_device, &gpu_props);
	limits = gpu_props.limits;

	vkGetPhysicalDeviceMemoryProperties(physical_device, &props);

	device = dev;
}

VkResult vkalloc_create_buffer(VkBufferCreateInfo *buffer_info, enum mem_type type, VkBuffer *buffer, struct vkalloc **alloc) {
	VkResult res;
	VkMemoryRequirements req;

	res = vkCreateBuffer(device, buffer_info, NULL, buffer);
	if(res != VK_SUCCESS)
		goto err_return;

	vkGetBufferMemoryRequirements(device, *buffer, &req);

	res = vkalloc(req, type, alloc);
	if(res != VK_SUCCESS)
		goto err_buffer;

	res = vkBindBufferMemory(device, *buffer, (*alloc)->memory, (*alloc)->offset);
	if(res != VK_SUCCESS)
		goto err_buffer;

	return VK_SUCCESS;

err_buffer:
	vkDestroyBuffer(device, *buffer, NULL);
err_return:
	return res;
}

VkResult vkalloc_create_image(VkImageCreateInfo *image_info, enum mem_type type, VkImage *image, struct vkalloc **alloc) {
	VkResult res;
	VkMemoryRequirements req;

	res = vkCreateImage(device, image_info, NULL, image);
	if(res != VK_SUCCESS)
		goto err_return;

	vkGetImageMemoryRequirements(device, *image, &req);

	res = vkalloc(req, type, alloc);
	if(res != VK_SUCCESS)
		goto err_image;

	res = vkBindImageMemory(device, *image, (*alloc)->memory, (*alloc)->offset);
	if(res != VK_SUCCESS)
		goto err_image;

	return VK_SUCCESS;

err_image:
	vkDestroyImage(device, *image, NULL);
err_return:
	return res;
}

VkResult vkalloc_map(struct vkalloc *vkalloc, void **data) {
	return vkMapMemory(device, vkalloc->memory, vkalloc->offset, vkalloc->size, 0, data);
}

void vkalloc_unmap(struct vkalloc *vkalloc) {
	vkUnmapMemory(device, vkalloc->memory);
}

VkResult vkalloc_free(struct vkalloc *vkalloc) {
	return mem_insert(vkalloc);
}

void vkalloc_destroy(void) {
	uint32_t i;
	struct mem_node *node;

	for(i = 0; i < pool_size; i++) {
		vkFreeMemory(device, pools[i], NULL);
	}

	node = head;

	while(node != NULL) {
		struct mem_node *tmp;

		tmp = node;
		node = node->next;

		free(tmp->alloc);
		free(tmp);
	}
}
