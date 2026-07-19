# VKGPGPU
A minimalist, lightweight framework for general-purpose computing on the GPU, written in pure C11 and Vulkan.
## Features

* Pure C: No linking with `stdc++`, no heavy libs (like VMA)
* Can be build with `cc */*.c`: No complex build systems
* Vulkan as API: Cross-platform hardware acceleration without being vendor locked

## Dependencies:
- [volk.h](https://github.com/zeux/volk): vulkan loading at runtime (included in `vendor/`)
- [vkalloc](https://sr.ht/~lmarz/vkalloc/): simple vulkan memory allocator (included in `vendor/`)
- [SPIRV-Reflect](https://github.com/KhronosGroup/SPIRV-Reflect): for shader reflection (included in `vendor/`)
- VulkanSDK: vulkan headers and etc.
- Any C11 compliant C compiler (gcc, clang, MSVC)
- Any SPIR-V compiler

