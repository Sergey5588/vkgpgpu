#include "../vendor/volk.h"
#include "../include/vkgpgpu.h"

#include "../vendor/vkalloc.h"

GpuBuffer gpu_buf_create(GpuContext *ctx, uint64_t size, uint8_t type) {
	GpuBuffer buf = {0};
	buf.ctx = ctx;
	buf.size = size;
	VkBufferCreateInfo bufferCI = {
		.sType =VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	vkalloc_create_buffer(&bufferCI,type, &buf.buffer,  (struct vkalloc**)&buf.alloc);
	return buf;
}
void gpu_buf_destroy(GpuBuffer *buf) {
	if(!buf) return;
	if(buf->alloc) {

		vkalloc_free(buf->alloc);
	}
	if(buf->buffer) {

		vkDestroyBuffer(buf->ctx->device,buf->buffer, NULL);
	}
	*buf = (GpuBuffer){0};
}
