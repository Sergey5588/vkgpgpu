#include "../include/vkgpgpu.h"
#include <stdio.h>
int main() {
	GpuContext *ctx = gpu_ctx_init();
	GpuBuffer *buf1 = gpu_buf_create(ctx, 1337*sizeof(float), MEM_STAGING);
	GpuBuffer *buf2 = gpu_buf_create(ctx, 1337*sizeof(float), MEM_STAGING);
	float *data1;
	float *data2;

	gpu_buf_map(buf1,(void**)&data1);
	gpu_buf_map(buf2, (void**)&data2);
	for(size_t i = 0; i < 1337; i++) data1[i] = i+1;

	GpuProgram *p = gpu_program_load(ctx, "./shaders/test.comp.spv");
	float alpha = 2.3f;
	float beta  = 3.7f;
	GpuCommand *cmd = gpu_command_begin(ctx);
	gpu_command_bind_buffer(cmd,0, buf1);
	gpu_command_bind_buffer(cmd,1, buf2);
	
	gpu_set(cmd, p, alpha);
	gpu_set(cmd, p, beta);

	gpu_command_dispatch(cmd, p, 7,1,1);
	gpu_command_submit(cmd); // wait for result

	printf("First values of buf1: %f, %f, %f\n", data1[0], data1[1], data1[2]);
	printf("First values of buf2: %f, %f, %f\n", data2[0], data2[1], data2[2]);


	//cleanup
	gpu_command_destroy(cmd);
	gpu_program_destroy(p);
	gpu_buf_destroy(buf1);
	gpu_buf_destroy(buf2);
	gpu_ctx_destroy(ctx);
	return 0;
}
