#define E_USE_NAMES
#include <errno.h>


const char*
iomanx_strerror (int err)
{
  if (err > -(sizeof (file_errors) / sizeof (file_errors[0])) && err < 0)
    return (error_to_string (err));
  else
    return ("");
}
