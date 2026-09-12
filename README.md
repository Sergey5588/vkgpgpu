# VKGPGPU
A minimalist, lightweight framework for general-purpose computing on the GPU, written in pure C11 on top of Vulkan.

## Quick start

Prebuilt binaries are available on the [Releases](../../releases) page.

Compile from source:
```bash
git clone https://github.com/Sergey5588/vkgpgpu
cd vkgpgpu
./build.sh run # build and run example, see './build.sh help' for usage

```
This will compile folowing code:

```c
GpuContext *ctx = gpu_ctx_init();
GpuBuffer *buffer = gpu_buf_create(ctx, 1337*sizeof(float), MEM_STAGING);
float *data;

gpu_buf_map(buffer,(void**)&data);
for(size_t i = 0; i < 1337; i++) data[i] = i+1; // fill data
printf("Sent to GPU: %f, %f, %f ...\n", data[0], data[1], data[2]);
gpu_buf_unmap(buffer);

GpuProgram *p = gpu_program_load(ctx, "./shaders/test.comp.spv"); // load shader from disk
float alpha = 67.0f;

GpuCommand *cmd = gpu_command_begin(ctx); // start recording
gpu_command_bind_buffer(cmd,0, buffer);

gpu_set(cmd, p, alpha); // push constant

gpu_command_dispatch(cmd, p, 7,1,1);
gpu_command_submit(cmd); // wait for result

gpu_buf_map(buffer,(void**)&data);
printf("Got: %f, %f, %f\n", data[0], data[1], data[2]);
// Expected 67.000000, 134.000000, 201.000000
gpu_buf_unmap(buffer);

//cleanup
gpu_command_destroy(cmd);
gpu_program_destroy(p);
gpu_buf_destroy(buffer);
gpu_ctx_destroy(ctx);
//...
```



## Features

* Pure C: No linking with `libstdc++`, no heavy libs (like VulkanMemoryAllocator)
* Single `build.sh`: No complex build systems
* Vulkan as API: Cross-platform hardware acceleration without being vendor-locked

## Dependencies:
- [volk.h](https://github.com/zeux/volk): vulkan loading at runtime (included in `vendor/`)
- [vkalloc](https://sr.ht/~lmarz/vkalloc/): simple vulkan memory allocator (included in `vendor/`)
- [SPIRV-Reflect](https://github.com/KhronosGroup/SPIRV-Reflect): for shader reflection (included in `vendor/`)
- [Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers): only at build time, vulkan-1.dll and libvulkan.so loads at runtime.
- Any C11 compliant C compiler (gcc, clang, MSVC)
- Any SPIR-V compiler

