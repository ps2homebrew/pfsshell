#include <string.h>
const char*
iomanx_strerror (int err)
{
	return strerror(-err);
}
