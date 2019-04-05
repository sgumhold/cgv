/* file added by Stefan Gumhold to include os specific implementations */

#ifdef _WIN32
#include "tif_win32.h"
#else
#include "tif_unix.h"
#endif