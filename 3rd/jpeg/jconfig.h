#ifdef WIN32
#ifdef _MSC_VER
#pragma warning (disable:4244)
#pragma warning (disable:4133)
#pragma warning (disable:4996)
#pragma warning (disable:4267)
#include "jconfig.vc"
#else
#include "jconfig.mingw"
#endif
#endif
