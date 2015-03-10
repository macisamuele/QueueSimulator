#include "server.h"
#include <stdio.h>
#include <math.h>
#include "errors_warnings_management.h"
#include "random_numbers.h"
#include "simulation_time.h"

struct server_t {
    Service_Distribution distribution;
    unsigned long number_of_threads;
    double *rates;
    double *probabilities;              ///probabilities[i]=P{select the branch i}
    double *cumulative_probabilities;   ///cumulative_probabilities[i] = P{select branch smaller or equal to i}

    unsigned long *n_gen; double *t_gen;
};


double server_M_getAverageNumberOfUserInTheSystem(Server server, double rho, unsigned long c);

void server_print(FILE *stream, Server server) {
    return_if_null("server", server,);
    unsigned long i;
    if (stream == NULL || stream == stdin) {
        stream = stdout;
    }
    fprintf(stream, "Type: ");
    switch(server->distribution) {
        case M:
            fprintf(stream, "M\n\tmu: %lf\n", server->rates[0]);
            break;
        case H:
            fprintf(stream, "H%ld\n", server->number_of_threads);
            for(i=0; i<server->number_of_threads; i++) {
                fprintf(stream, "\tBranch %ldd with probability %lf and rate %lf\n", i+1, server->probabilities[i], server->rates[i]);
            }
            break;
        case E:
            fprintf(stream, "E%ld\n", server->number_of_threads);
            for(i=0; i<server->number_of_threads; i++) {
                fprintf(stream, "\tTask %ld with rate %lf\n", i+1, server->rates[i]);
            }
            break;
        case D:
            fprintf(stream, "D\n\tmu: %lf\n", server->rates[0]);
            break;
    }
}
const char *server_getUsage() {
    return "The initialization of the server from the command line have the following structure:\n\tM mu\n\tH n mu_1 mu_2 ... mu_n  alpha_1 alpha_2 ... alpha_{n-1}\n\tE n mu_1 mu_2 ... mu_n\n\tD mu";
}
Server server_init(Service_Distribution distribution, unsigned long number_of_threads, double *rates, double *probabilities) {
    double *cumulative_probabilities = NULL;
    unsigned long i;
    Server server;
    if(probabilities!=NULL) {
        _alloc(cumulative_probabilities, double *, number_of_threads);
        cumulative_probabilities[0]=probabilities[0];
        for(i=1; i<number_of_threads; i++) {
            cumulative_probabilities[i]=cumulative_probabilities[i-1]+probabilities[i];
        }
    }
    _alloc(server, Server, 1);
    server->distribution=distribution;
    server->rates=rates;
    server->number_of_threads=number_of_threads;
    server->probabilities=probabilities;
    server->cumulative_probabilities=cumulative_probabilities;
    _alloc(server->n_gen, unsigned long *, number_of_threads);
    _alloc(server->t_gen, double *, number_of_threads);
    for(i=0; i<number_of_threads; i++) {
        server->n_gen[i]=0;
        server->t_gen[i]=0;
    }
    return server;
}
Server server_keyboard_init() {
    double *rates, *probabilities, cumulative_probability;
    unsigned long number_of_threads;
    char cmd[256+1]={0};
    unsigned long i;
    cumulative_probability = 0;
    number_of_threads = 0;
    rates = NULL;
    probabilities = NULL;
    Service_Distribution distribution;
    printf("Insert service distribution (M, H, E, D): ");
    scanf("%s", cmd);
    if(strlen(cmd)!=1) {
        error_reporting("wrong service distribution");
    }
    switch(cmd[0]) {
        case 'M':
            distribution=M;
            number_of_threads=1;
            printf("Insert the service rate: ");
            _alloc(rates, double *, 1);
            scanf("%lf", &rates[0]);
            break;
        case 'H':
            distribution=H;
            printf("Insert the number of branches: ");
            scanf("%ld", &number_of_threads);
            _alloc(rates, double *, number_of_threads);
            _alloc(probabilities, double *, number_of_threads);
            for(i=0; i<number_of_threads-1; i++) {
                printf("Insert the rate for the branch no. %ld: ", i+1);
                scanf("%lf", &rates[i]);
                printf("Insert the probability to enter in the branch no. %ld: ", i+1);
                scanf("%lf", &probabilities[i]);
                cumulative_probability+=probabilities[i];
            }
            printf("Insert the rate for the branch no. %ld: ", i+1);
            scanf("%lf", &rates[i]);
            probabilities[i]=1-cumulative_probability;
            break;
        case 'E':
            distribution=E;
            printf("Insert the number of serial tasks: ");
            scanf("%ld", &number_of_threads);
            _alloc(rates, double *, number_of_threads);
            for(i=0; i<number_of_threads-1; i++) {
                printf("Insert the rate for the task no. %ld: ", i+1);
                scanf("%lf", &rates[i]);
            }
            break;
        case 'D':
            distribution=D;
            number_of_threads=1;
            printf("Insert the service rate: ");
            _alloc(rates, double *, 1);
            scanf("%lf", &rates[0]);
            break;
        default:
            error_reporting("wrong service distribution");
            break;
    }
    return server_init(distribution, number_of_threads, rates, probabilities);
}
Server server_command_line_init(int argc, char *argv[], int *readed) {
    double *rates, *probabilities;
    unsigned long number_of_threads;
    Service_Distribution distribution;
    int _readed;
    unsigned int i;
    double cumulative_probability;
    if(argc<=0) {
        error_reporting(server_getUsage());
    }
    cumulative_probability = 0;
    number_of_threads = 0;
    rates=NULL;
    probabilities=NULL;
    if(argv[0][1]!='\0') {
        fprintf(stderr, "Available service distribution are: M, H, E, G\n");
        return NULL;
    }
    switch(argv[0][0]) {
        case 'M':
            distribution=M;
            if(argc>1) {
                number_of_threads=1;
                _alloc(rates, double *, number_of_threads);
                if(sscanf(argv[1], "%lf", &rates[0])!=1) {
                    fprintf(stderr, "Wrong parameter\n");
                    return NULL;
                }
            }
            else {
                fprintf(stderr, "Wrong number of parameters\n");
                return NULL;
            }
            _readed=2;
            break;
        case 'H': //example H 2 2 3 0.2  //2 branches the first with rate 2 and probability 0.2 and the other with rate 3 and probability 0.2
                  //I aspect to have 2 parameters that describes the H and the n and then n+n-1 others parameters
            distribution=H;
            if(argc>1) {
                if(sscanf(argv[1], "%ld", &number_of_threads)!=1) {
                    fprintf(stderr, "Wrong parameter\n");
                    return NULL;
                }
                if((unsigned)argc>1+number_of_threads+number_of_threads-1) {
                    _alloc(rates, double *, number_of_threads);
                    _alloc(probabilities, double *, number_of_threads);
                    for(i=0; i<number_of_threads; i++) {
                        if(sscanf(argv[2+i], "%lf", &rates[i])!=1) {
                            fprintf(stderr, "Wrong parameter\n");
                            return NULL;
                        }
                    }
                    for(i=0; i<number_of_threads-1; i++) {
                        if(sscanf(argv[2+number_of_threads+i], "%lf", &probabilities[i])!=1) {
                            fprintf(stderr, "Wrong parameter\n");
                            return NULL;
                        }
                        cumulative_probability+=probabilities[i];
                    }
///CONTROLLARE IL CASO LIMITE IN CUI HO H1
                    probabilities[i] = 1-cumulative_probability;
                }
                else {
                    fprintf(stderr, "Wrong number of parameters\n");
                    return NULL;
                }
            }
            else {
                fprintf(stderr, "Wrong number of parameters\n");
                return NULL;
            }
            _readed=2+number_of_threads+number_of_threads-1;
            break;
        case 'E':  //E n m1 m2 .. m2 (E n and n parameters)
            distribution=E;
            if(argc>1) {
                if(sscanf(argv[1], "%ld", &number_of_threads)!=1) {
                    fprintf(stderr, "Wrong parameter\n");
                    return NULL;
                }
                if((unsigned)argc>1+number_of_threads) {
                    _alloc(rates, double *, number_of_threads);
                    for(i=0; i<number_of_threads; i++) {
                        if(sscanf(argv[2+i], "%lf", &rates[i])!=1) {
                            fprintf(stderr, "Wrong parameter\n");
                            return NULL;
                        }
                    }
                }
                else {
                    fprintf(stderr, "Wrong number of parameters\n");
                    return NULL;
                }
            }
            else {
                fprintf(stderr, "Wrong number of parameters\n");
                return NULL;
            }
            _readed=2+number_of_threads;
            break;
        case 'D':  // D rate  => service_time = 1/rate
            distribution=D;
            if(argc>1) {
                number_of_threads=1;
                _alloc(rates, double *, number_of_threads);
                if(sscanf(argv[1], "%lf", &rates[0])!=1) {
                    fprintf(stderr, "Wrong parameter\n");
                    return NULL;
                }
            }
            else {
                fprintf(stderr, "Wrong number of parameters\n");
                return NULL;
            }
            _readed=2;
            break;
        default:
            fprintf(stderr, "UNKNOUN DISTRIBUTION");
            break;
    }
    if(number_of_threads>0) {
        if(readed!=NULL) {
            *readed=_readed;
        }
        return server_init(distribution, number_of_threads, rates, probabilities);
    }
    return NULL;
}
Time server_getServiceTime(Server server) {
    return_if_null("server", server, WRONG_TIME);
    static double u;
    static Time duration;
    static unsigned long branch;
    branch=0;
    duration = WRONG_TIME;
    switch(server->distribution) {
        case M:
            duration = negexp(1.0/server->rates[0]);
            server->n_gen[0]++;
            server->t_gen[0]+=duration;
            break;
        case H:
            u = uniform(0, 1);
            for(branch=0; branch<server->number_of_threads && server->cumulative_probabilities[branch]<u; branch++){
                ;
            }
            duration = negexp(1.0/server->rates[branch]);
            server->n_gen[branch]++;
            server->t_gen[branch]+=duration;
            break;
        case E:
            duration=0;
            u=0;
            for(branch=0; branch<server->number_of_threads; branch++) {
                u=negexp(1.0/server->rates[branch]);
                duration+=u;
                server->n_gen[branch]++;
                server->t_gen[branch]+=u;
            }
            break;
        case D:
            duration = 1.0/server->rates[0];
            server->n_gen[0]++;
            server->t_gen[0]+=duration;
            break;
    }
    return duration;
}
double server_getAverageServiceRate(Server server) {
    return_if_null("server", server, -1);
    unsigned long i;
    double avg_rate = -1;
    switch(server->distribution) {
        case M: case E: case D:
            avg_rate=0;
            for(i=0; i<server->number_of_threads; i++) {
                avg_rate+=1.0/server->rates[i];
            }
            avg_rate=1/avg_rate;
            break;
        case H:
            avg_rate=0;
            for(i=0; i<server->number_of_threads; i++) {
                avg_rate+=server->probabilities[i]/server->rates[i];
            }
            avg_rate=1/avg_rate;
            break;
    }
    return avg_rate;
}
double server_getAverageOfServiceTime(Server server) {
    return 1.0 / server_getAverageServiceRate(server);
}
double server_getVarianceOfServiceTime(Server server) {
    return_if_null("server", server, 0);
    double variance;
    unsigned long i, j;
    switch(server->distribution) {
        case M:
            //variance = 1/mu^2
            for(i=0; i<server->number_of_threads; i++) {
                variance = 1 / ( server->rates[0] * server->rates[0]);
            }
            break;
        case H:
            //variance = sum(i, 1, n) (alpha_i/mu_i)^2 + sum(i, 1, n) sum(j, 1, n) alpha_i*alpha_j*(1/mu_i-1/mu_j)^2
            variance = 0;
            for(i=0; i<server->number_of_threads; i++) {
                variance += server->probabilities[i] / server->rates[i];
            }
            variance = variance*variance;
            for(i=0; i<server->number_of_threads; i++) {
                for(j=0; j<server->number_of_threads; j++) {
                    variance += server->probabilities[i]*server->probabilities[j]*(1/server->rates[i]-1/server->rates[j])*(1/server->rates[i]-1/server->rates[j]);
                }
            }
            break;
        case E:
            //variance = sum(i, 1, n) 1/mu_i^2
            variance = 0;
            for(i=0; i<server->number_of_threads; i++) {
                variance += 1.0 / (server->rates[i] * server->rates[i]);
            }
            break;
        case D:
            //variance = 0
            variance = 0;
            break;
    }
    return variance;
}
double server_getSquareCoefficientOfVariation(Server server) {
    return_if_null("server", server, 0);
    static double avg, variance;
    variance = server_getVarianceOfServiceTime(server);
    avg=server_getAverageOfServiceTime(server);
    return variance/(avg*avg);
}
double server_getAverageResponseTime(Server server, unsigned long number_of_servers, double rho) {
    return_if_null("server", server, -1);
    if(number_of_servers!=1) {
        if(server->distribution!=M) {
            warning_reporting("The Pollaczek-Khnichine formula holds only the case of M/G/1");
            return -1;
        }
    }
    return server_getAverageNumberOfUserInTheSystem(server, number_of_servers, rho)/(rho*number_of_servers*server_getAverageServiceRate(server));  //rho*number_of_server*server_getAverageServiceRate(server)=lambda
}
double server_getAverageNumberOfUserInTheSystem(Server server, unsigned long number_of_servers, double rho) {
// apply the Pollaczek-Khinchinhe formula => it holds only for M/G/1
    return_if_null("server", server, -1);
    if(number_of_servers!=1) {
        if(server->distribution==M) {
            return server_M_getAverageNumberOfUserInTheSystem(server, rho, number_of_servers);
        }
        else {
            warning_reporting("The Pollaczek-Khnichine formula holds only the case of M/G/1");
            return -1;
        }
    }
   //E[L]=rho+(rho^2*(1+C_s^2))/(2*(1-rho))
   return rho+((rho * rho)*(1+server_getSquareCoefficientOfVariation(server)))/(2*(1-rho));
}
double server_getPI0(Server server, double rho, unsigned long c) {
//PI_0 = 1/(sum(k, 0, c-1) (c*rho)^k/k! + ((c*rho)^c)/(c!*(1-rho)))
    return_if_null("server", server, -1);
    static unsigned long k, factorial;
    static double result, power;
    if(c==1) {
        return 1-rho;
    }
    if(server->distribution!=M) {
        warning_reporting("The idle system probability can be evaluated only for Markovian services if are present more then one server");
        return -1;
    }
    result = 0;
    power = 1;
    factorial = 1;
    for (k = 0; k < c; k++) {
        if (k != 0) {
            factorial *= k;
        }
        result += power / factorial;
        power *= c * rho;
    }
    factorial *= k;
    result += power / factorial * 1 / (1 - rho);
    result = 1 / result;
    return result;
}
/**
 * Description: evaluate the theorical waiting time using the Erlang formula
 * Return: theorical waiting time for the system
 */
double server_M_getAverageNumberOfUserInTheSystem(Server server, double rho, unsigned long c) {
//E[L] = PI_0*(c*rho)^c/c!*rho/(1-rho)^2 + c*rho
    return_if_null("server", server, -1);
    static unsigned long k, factorial;
    static double result, power;
    power = 1;
    factorial = 1;
    for (k = 0; k < c; k++) {
        if (k != 0) {
            factorial *= k;
        }
        power *= c * rho;
    }
    factorial *= k;
    result = server_getPI0(server, rho, c) * power / factorial * rho / ((1 - rho) * (1 - rho)) + c * rho;
    return result;
}
double server_get_loss_probability(Server server, unsigned long number_of_server, long waiting_line_size, double lambda) {
    if(waiting_line_size==-1) {
        return 0;
    }
    else if(number_of_server==1) {
        return lambda/(lambda+server_getAverageServiceRate(server));
    }
    else if(server->distribution == M) {
        return -1;
    }
    else {
        return -1;
    }
}
void server_statistics(Server server) {
    unsigned long i, t=0;
    printf("%4s | %10s | %20s | %8s | %10s\n", "i", "n_gen", "t_gen", "%% n_gen", "rate_gen");
    for(i=0; i<server->number_of_threads; i++) {
        t+=server->n_gen[i];
    }
    for(i=0; i<server->number_of_threads; i++) {
        printf("%4ld | %10ld | %20.8lf | %8.4lf | %10lf\n", i, server->n_gen[i], server->t_gen[i], 100.0*server->n_gen[i]/t, 1.0*server->n_gen[i]/server->t_gen[i]);
    }
}

void server_free(Server server) {
	return_if_null("server", server,);
	free(server->rates);
	free(server->probabilities);
	free(server->cumulative_probabilities);
	free(server->n_gen);
	free(server->t_gen);
	free(server);
}
