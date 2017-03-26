#pragma once

#include <stdlib.h>

int parse_line(char *line, /* modified */
               char *(*tokens)[],
               size_t *count);
