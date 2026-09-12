#include "../../include/vkgpgpu.h"
#include <stdio.h>
int main() {
	GpuContext *ctx = gpu_ctx_init();
	GpuBuffer *buffer = gpu_buf_create(ctx, 1337*sizeof(float), MEM_STAGING);
	float *data;
	gpu_buf_map(buffer,(void**)&data);
	for(size_t i = 0; i < 1337; i++) data[i] = i+1; // fill data
	printf("Sent to GPU: %f, %f, %f ...\n", data[0], data[1], data[2]);
	
	GpuProgram *p = gpu_program_load(ctx, "./shaders/test.comp.spv"); // load shader from disk
	float alpha = 67.0f;

	GpuCommand *cmd = gpu_command_begin(ctx); // start recording
	gpu_command_bind_buffer(cmd,0, buffer);
	
	gpu_set(cmd, p, alpha); // push constant

	gpu_command_dispatch(cmd, p, 7,1,1);
	gpu_command_submit(cmd); // wait for result

	printf("Got: %f, %f, %f\n", data[0], data[1], data[2]); // Expected 67.000000, 134.000000, 201.000000

	//cleanup
	gpu_command_destroy(cmd);
	gpu_program_destroy(p);
	gpu_buf_destroy(buffer);
	gpu_ctx_destroy(ctx);
	return 0;
}
