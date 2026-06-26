#include "../include/vkgpgpu.h"
#include <stdio.h>
int main() {
	GpuContext *ctx = gpu_ctx_init();
	GpuBuffer *buf1 = gpu_buf_create(ctx, 1337*sizeof(uint32_t),0);
	GpuBuffer *buf2 = gpu_buf_create(ctx, 1337*sizeof(uint32_t), 0);
	uint32_t *data1;
	uint32_t *data2;

	gpu_buf_map(buf1,(void**)&data1);
	for(uint32_t i = 0; i < 1337; i++) data1[i] = i;
	gpu_buf_unmap(buf1);
	GpuKernel *k = gpu_kernel_create(ctx, "./shaders/test.comp.spv", 2, buf1, buf2);
	gpu_kernel_dispatch(k,6,1,1);
	gpu_buf_map(buf2, (void**)&data2);
	gpu_buf_map(buf1, (void**)&data1);
	printf("First values of buf1: %u, %u, %u\n", data1[0], data1[1], data1[2]);
	printf("First values of buf2: %u, %u, %u\n", data2[0], data2[1], data2[2]);
	gpu_buf_unmap(buf2);
	gpu_buf_unmap(buf1);


	//cleanup
	gpu_kernel_destroy(k);
	gpu_buf_destroy(buf1);
	gpu_buf_destroy(buf2);
	gpu_ctx_destroy(ctx);
	return 0;
}
