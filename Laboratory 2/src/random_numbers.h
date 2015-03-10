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

#ifndef RANDOM_NUMBERS_H_INCLUDED
#define RANDOM_NUMBERS_H_INCLUDED

void set_seed(long _seed);
long get_seed();
void rnd32();
double uniform(double a, double b);
double uniform01();
double negexp(double mean);
double pareto(double alpha, double kappa);
double pareto_m(double alpha, double mean);
int poisson(double alpha);
int geometric0(double mean);
int geometric1(double mean);
int geometric_trunc1(double mean, int max_len);
int trunc_exp(double mean, long length);

#endif // RANDOM_NUMBERS_H_INCLUDED
