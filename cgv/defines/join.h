#pragma once

/// join two arguments
#define CGV_DEFINES_JOIN( X, Y ) CGV_DEFINES_DO_JOIN( X, Y )
#define CGV_DEFINES_DO_JOIN( X, Y ) CGV_DEFINES_DO_JOIN2(X,Y)
#define CGV_DEFINES_DO_JOIN2( X, Y ) X##Y
