#include <stdio.h>
#include <stdarg.h>
#include "dbg.h"


/* DONE */
void _dbg_printf(const char *mask, ...)
{
    va_list va;
    va_start(va, mask);
    vprintf(mask, va);
    va_end(va);
}
