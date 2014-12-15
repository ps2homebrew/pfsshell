#include "types.h"
#include <ctype.h>
#include <stdlib.h>


/* maybe DONE */

/* have absolutely no idea what the following one should do ;-) */
unsigned char
look_ctype_table (char character)
{
  if (isdigit (character))
    return (0x04);
  else if (isalpha (character))
    return (0x02);
  else
    return (0);
}
