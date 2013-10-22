#include "thread.h"

#ifdef _WIN32
#include <cgv/config/cpp_version.h>
#ifdef CPP11
#define USE_STD_THREAD
#endif
#else
#define USE_STD_THREAD
#endif

#ifdef USE_STD_THREAD
#include "thread_std_thread.h"
#else
#include "thread_pthread.h"
#endif
