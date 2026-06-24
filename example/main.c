#define _GNU_SOURCE // for usleep
#include "../include/vkgpgpu.h"
#include <unistd.h> // for usleep, just for tests
int main() {
	GpuContext *ctx = gpu_ctx_init();
	GpuBuffer *buf1 = gpu_buf_create(ctx, 1337*sizeof(int32_t),0);
	GpuBuffer *buf2 = gpu_buf_create(ctx, 69420*sizeof(int32_t), 0);
	int32_t* data1;
	int32_t* data2;

	gpu_buf_map(buf1,(void**)&data1);
	gpu_buf_map(buf2,(void**)&data2);
	data1[0] = 1123123123;
	usleep(60000000); // simulate some hard work
	gpu_buf_unmap(buf1);
	gpu_buf_unmap(buf2);
	gpu_buf_destroy(buf1);
	gpu_buf_destroy(buf2);
	gpu_ctx_destroy(ctx);
	return 0;
}
