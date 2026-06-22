#include "../include/vkgpgpu.h"


int main() {
	GpuContext ctx = gpu_ctx_init();

	gpu_ctx_destroy(&ctx);
	return 0;
}
