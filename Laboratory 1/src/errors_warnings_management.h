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

#ifndef ERRORS_WARNING_MANAGEMENT_H_INCLUDED
#define ERRORS_WARNING_MANAGEMENT_H_INCLUDED

#include <string.h>  //for strrchr
#include <stdlib.h>  //for exit and malloc

#ifdef _WIN32
    #define remove_path(file) (strrchr(file, '\\')==NULL ? file : strrchr(file, '\\')+1)
#else
    #define remove_path(file) (strrchr(file, '/')==NULL ? file : strrchr(file, '/')+1)
#endif
#define error_reporting(str) { fprintf(stderr, "Error on line %d of file %s (function %s) \"%s\"\n", __LINE__, remove_path(__FILE__), __func__, str); exit(EXIT_FAILURE); }
#define warning_reporting(str) { fprintf(stderr, "Warning on line %d of file %s (function %s) \"%s\"\n", __LINE__, remove_path(__FILE__), __func__, str); }
#define precedence(a, b) ((a)>(b) ? 1 : ((a)<(b) ? -1 : 0))
#define _alloc(variable, type, number) { if((variable=(type)malloc(number*sizeof(*(variable))))==NULL) { error_reporting("Allocation Error"); } }
#define error_null(name, variable) if(variable==NULL) { error_reporting(name" is NULL"); }
#define warning_null(name, variable) if(variable==NULL) { warning_reporting(name" is NULL"); }
#define return_if_null(name, variable, return_value) if(variable==NULL) { warning_reporting(name" is NULL");  return return_value;}

#endif // ERRORS_WARNING_MANAGEMENT_H_INCLUDED
