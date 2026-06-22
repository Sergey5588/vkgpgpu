#ifndef __TODO_H
#define __TODO_H
#include <stdio.h>
#define TODO(msg) do { \
	fprintf(stderr, "TODO at %s:%d: %s\n", __FILE__, __LINE__, msg); \
} while(0) \

#endif
