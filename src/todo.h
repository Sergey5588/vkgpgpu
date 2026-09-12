#ifndef _TODO_H
#define _TODO_H
#include <stdio.h>
#ifdef VKGPGPU_DEBUG
#define TODO(msg) do { \
	fprintf(stderr, "TODO at %s:%d: %s\n", __FILE__, __LINE__, msg); \
} while(0)
#else
#define TODO(msg)
#endif

#endif
