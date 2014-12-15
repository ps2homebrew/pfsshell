#if !defined (_UTIL_H)
#  define _UTIL_H

#include <stdlib.h>

int parse_line (char *line, /* modified */
		char *(*tokens)[],
		size_t *count);

#endif /* _UTIL_H defined? */
