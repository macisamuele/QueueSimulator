#ifndef SIMULATION_TIME_H_INCLUDED
#define SIMULATION_TIME_H_INCLUDED

typedef double Time;

#include <float.h>
#ifdef DBL_MAX
#define MAX_TIME    DBL_MAX
#else
#define MAX_TIME    1.797693e+308
#endif
#define WRONG_TIME -1.0

#endif // SIMULATION_TIME_H_INCLUDED
