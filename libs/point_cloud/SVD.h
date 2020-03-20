#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "defs_and_types.h"
#include "lib_begin.h"

static double PYTHAG(double a, double b);
int dsvd(float **a, int m, int n, float *w, float **v);

#include <cgv/config/lib_end.h>
