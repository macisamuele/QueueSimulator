#ifndef RANDOM_NUMBERS_H_INCLUDED
#define RANDOM_NUMBERS_H_INCLUDED

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
