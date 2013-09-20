#pragma once

#include <cgv/config/cpp_version.h>

#ifdef CPP11
#define REFLECT_IN_CLASS_NAMESPACE
#else
#define REFLECT_TRAITS_WITH_DECLTYPE
#endif

