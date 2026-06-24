#include "../include/vkgpgpu.h"


int main() {
	GpuContext ctx = gpu_ctx_init();
	GpuBuffer  buf = gpu_buf_create(&ctx, 1337,0);

	gpu_buf_destroy(&buf);
	gpu_ctx_destroy(&ctx);
	return 0;
}
