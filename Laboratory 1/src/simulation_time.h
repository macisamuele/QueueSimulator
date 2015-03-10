/*
 * Copyright 2014 Samuele Maci (macisamuele@gmail.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

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
