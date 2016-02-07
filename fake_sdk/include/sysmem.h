
#pragma once

#include <stdlib.h>

#define AllocSysMemory(x, y, z) malloc(y)

static inline int FreeSysMemory(void *ptr) {
	free(ptr);
	return 0;
}
