#include "../include/vkgpgpu.h"
int main() {
	GpuContext *ctx = gpu_ctx_init();
	GpuBuffer *buf1 = gpu_buf_create(ctx, 1337*sizeof(uint32_t),0);
	GpuBuffer *buf2 = gpu_buf_create(ctx, 1337*sizeof(uint32_t), 0);
	uint32_t *data1;
	uint32_t *data2;

	gpu_buf_map(buf1,(void**)&data1);
	gpu_buf_map(buf2,(void**)&data2);
	data1[0] = 1123123123;
	gpu_buf_unmap(buf1);
	gpu_buf_unmap(buf2);
	GpuKernel *k = gpu_kernel_create(ctx, "./shaders/copy.comp.spv", 2, buf1, buf2);
	gpu_buf_destroy(buf1);
	gpu_buf_destroy(buf2);
	gpu_ctx_destroy(ctx);
	return 0;
}
