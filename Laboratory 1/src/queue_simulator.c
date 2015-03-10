#include "queue_simulator.h"
#include <stdio.h>
#include "errors_warnings_management.h"
#include "event.h"
#include "simulation_time.h"
#include "server.h"
#include "random_numbers.h"

struct queue_simulator_t {
//input parameters
	double lambda;						//arrival rate
	Server server;                      //information about the kind of the server, allow to generate the correct service time
	Time maximum;						//maximum time to simulate
	unsigned long number_of_servers;	//number of servers in the system
	unsigned long population_size;      //maximum number of the users that can enter in the system (0 means infinit number of users)
	long waiting_line_size;    //maximum number of the users that can wait for a service
//internal variables
	EventList scheduled_events, waiting_events;
	Time current;			    		//current simulation time
	unsigned long busy_servers; 		//number of server currently busy
	Time last_event_time; 				//time in which occur the last event
	unsigned long total_users; 			//number of users currently in the system
//measures
	unsigned long number_of_users;		//number of users generated
	unsigned long number_of_services;	//number of ended services
	Time cumulative_inter_arrival_time;	//summation of all the generated inter arrival time
	Time cumulative_service_time;		//summation of all the generated service time
	Time cumulative_waiting_time;		//summation of all the waited time of all the events putted in service
	Time cumulative_free_system_time;	//summation of all the time in which the system was empty (0 user inside)
	unsigned long number_of_losses;     //count the number of event that was lost (no space on the waiting line)
};

double ABS(double X) {
	if(X<0) {
		X=-X;
	}
	return X;
}
#define inter_arrival_time(sim) negexp(1.0/sim->lambda)
#define service_time(sim) server_getServiceTime(sim->server)
#define schedule_event(sim, event) eventlist_insert(sim->scheduled_events, event);
#define have_schedulable_events(sim) (!eventlist_is_empty(sim->scheduled_events))
#define have_space_in_waiting_line(sim) (sim->waiting_line_size<0 || sim->total_users==0 || sim->total_users-sim->busy_servers<(unsigned long)sim->waiting_line_size)
#define have_users_to_be_generated(sim) (sim->population_size==0 || sim->number_of_users<sim->population_size)
#define get_rho(sim) (theoretic_rho(sim))

#define theoretic_average_inter_arrival_time(sim) (1.0/sim->lambda)
#define theoretic_average_waiting_time(sim) (server_getAverageResponseTime(sim->server, sim->number_of_servers, get_rho(sim))-1.0/server_getAverageServiceRate(sim->server))
#define theoretic_average_service_time(sim) (1.0/server_getAverageServiceRate(sim->server))
#define theoretic_average_busy_servers(sim) (sim->number_of_servers * theoretic_rho(sim))
#define theoretic_average_response_time(sim) (server_getAverageResponseTime(sim->server, sim->number_of_servers, get_rho(sim)))
#define theoretic_average_number_of_users(sim) (server_getAverageNumberOfUserInTheSystem(sim->server, sim->number_of_servers, get_rho(sim)))
//#define theoretic_probability_of_idle_system(sim) (1-theoretic_rho(sim))
#define theoretic_probability_of_idle_system(sim) server_getPI0(sim->server, theoretic_rho(sim), sim->number_of_servers)
//#define theoretic_loss_probability(sim) (sim->waiting_line_size == -1 ? 0 : ((sim->number_of_servers==1 ? theoretic_average_service_time(sim)/(theoretic_average_inter_arrival_time(sim)+theoretic_average_service_time(sim)) : (double)-1)))
// avg_service_rate/(avg_arrival_rate+avg_service_rate) = avg_arrival_time/(avg_arrival_time+avg_service_time)
#define theoretic_loss_probability(sim) (server_get_loss_probability(sim->server, sim->number_of_servers, sim->waiting_line_size, sim->lambda))
#define theoretic_rho(sim) (sim->lambda / (sim->number_of_servers * server_getAverageServiceRate(sim->server)))

#define simulated_average_inter_arrival_time(sim) (sim->cumulative_inter_arrival_time / sim->number_of_users)
#define simulated_average_waiting_time(sim) (sim->cumulative_waiting_time / (sim->number_of_services + sim->busy_servers))
#define simulated_average_service_time(sim) (sim->cumulative_service_time / (sim->number_of_services + sim->busy_servers))
#define simulated_average_busy_servers(sim) (sim->cumulative_service_time / (sim->current))
#define simulated_average_response_time(sim) (simulated_average_waiting_time(sim)+simulated_average_service_time(sim))
#define simulated_average_number_of_users(sim) (simulated_average_response_time(sim)*sim->lambda)
#define simulated_probability_of_idle_system(sim) (sim->cumulative_free_system_time/sim->current)
#define simulated_loss_probability(sim) ((double)sim->number_of_losses/sim->number_of_users)
#define simulated_rho(sim) ((1.0 / simulated_average_inter_arrival_time(sim)) / (sim->number_of_servers * 1.0 / simulated_average_service_time(sim)))

void generate_new_arrival(Queue_Simulator sim);
void generate_departure(Queue_Simulator sim, Event event);
void manage_arrival(Queue_Simulator sim, Event event);
void manage_departure(Queue_Simulator sim, Event event);
double queue_simulator_get_PI_0(Queue_Simulator sim);
Time _theoretic_average_waiting_time(Queue_Simulator sim);

#define PRINT_double(V)  printf(#V " = %lf \n", sim->V )
#define PRINT_long(V)  printf(#V " = %ld \n", sim->V )
#define PRINT_macro_double(V)  printf(#V " = %.20lf \n", V )
#define PRINT_macro_long(V)  printf(#V " = %d \n", V )
void queue_simulator_statistics(Queue_Simulator sim) {
#ifdef STAT
    printf("STATISTICS\n");
	PRINT_long(busy_servers);
	PRINT_double(last_event_time);
	PRINT_long(total_users);
	PRINT_long(number_of_users);
	PRINT_long(number_of_services);
	PRINT_double(cumulative_inter_arrival_time);
	PRINT_double(cumulative_service_time);
	PRINT_double(cumulative_waiting_time);
	PRINT_double(cumulative_free_system_time);
	PRINT_long(number_of_losses);
    server_statistics(sim->server);
    printf("AVG Service Time = %.20lf\n", server_getAverageOfServiceTime(sim->server));
    printf("VAR Service Time = %.20lf\n", server_getVarianceOfServiceTime(sim->server));
#else
    sim=sim;
#endif
}
/**
 * Description: allocate a simulation of a M/M/k system
 * Return: the new simulation
 */
Queue_Simulator queue_simulator_init(unsigned long number_of_servers,
		double lambda, Server server, Time maximum, unsigned long population_size, long waiting_line_size) {
	Queue_Simulator sim;
	_alloc(sim, Queue_Simulator, 1);
	sim->lambda = lambda;
	sim->server = server;
	sim->maximum = maximum;
	sim->number_of_servers = number_of_servers;
	sim->population_size = population_size;
	sim->waiting_line_size = waiting_line_size;
	sim->current = 0;
	sim->scheduled_events = eventlist_init();
	sim->waiting_events = eventlist_init();
	sim->total_users = 0;
	sim->busy_servers = 0;
	sim->last_event_time = 0.0;
	sim->number_of_services = 0;
	sim->cumulative_inter_arrival_time = 0.0;
	sim->cumulative_service_time = 0.0;
	sim->cumulative_waiting_time = 0.0;
	sim->number_of_users = 0;
	sim->cumulative_free_system_time = 0.0;
    sim->number_of_losses = 0;
	return sim;
}
/**
 * Description: allocate a simulation of a M/M/k system requiring from the keyboard (stdin) the initial parameters
 * Return: the new simulation
 */
Queue_Simulator queue_simulator_keyboard_init() {
    Server server;
	double lambda;
	Time maximum;
	unsigned long number_of_servers;
	unsigned long population_size;
    long waiting_line_size;
	printf("Insert the number of servers: ");
	scanf("%ld", &number_of_servers);
	printf("Insert the value of the arrival rate, lambda: ");
	scanf("%lf", &lambda);
    server = server_keyboard_init();
	printf("Insert the value of the maximum simulation time: ");
	scanf("%lf", &maximum);
	printf("Insert the maximum number of users that can enter in the system (0 for infinit number of users): ");
	scanf("%ld", &population_size);
	printf("Insert the size of the waiting line (-1 for infinit waiting line): ");
	scanf("%ld", &waiting_line_size);
	return queue_simulator_init(number_of_servers, lambda, server, maximum, population_size, waiting_line_size);
}
/**
 * Description: allocate a simulation of a M/M/k system requiring from the command line parameters (passed)
 * Parameters:
 *             - argc is the number of the parameters
 *             - argv is the array of parameters (the first is yet an uneful parameter
 * Return: the new simulation if the parameters are correct otherwise queue_simulator_keyboard_init()
 */
Queue_Simulator queue_simulator_command_line_init(int argc, char *argv[]) {
	double lambda;
    Server server;
	Time maximum;
	unsigned long number_of_servers;
	unsigned long population_size;
	long waiting_line_size;
    int n;
	int error=0;
	if(argc>5) {
        if(!sscanf(argv[0], "%ld", &number_of_servers)) { error=1; }
        if(!sscanf(argv[1], "%lf", &lambda)) { error=1; }
        if((server = server_command_line_init(argc-2, argv+2, &n)) == NULL) { error=1; }
        if(argc>=2+n+3) {
            if(!sscanf(argv[2+n], "%lf", &maximum)) { error=1; }
            if(!sscanf(argv[2+n+1], "%ld", &population_size)) { error=1; }
            if(!sscanf(argv[2+n+2], "%ld", &waiting_line_size)) { error=1; }
        }
        else {
            error=1;
        }
    }
    else {
        error=1;
    }
    if(error) {
		fprintf(stderr, "Wrong number of command line paramenters.\n\tUsage: command number_of_servers lambda <server> maximum_simulation_time population_size waiting_line_size\npopulation_size = 0 means infinit population\nwaiting_line_size<0 means infinit waiting line\n");
        fprintf(stderr, "%s\n\n", server_getUsage());
        warning_reporting("Wrong input line parameters. The initialization continue through the keyboard.");
		return queue_simulator_keyboard_init();
    }
    return queue_simulator_init(number_of_servers, lambda, server, maximum, population_size, waiting_line_size);
}
/**
 * Description: generate a new arrival in the system
 */
void generate_new_arrival(Queue_Simulator sim) {
	return_if_null("sim", sim,);
	static double duration;
	if(have_users_to_be_generated(sim)) {
	    duration = inter_arrival_time(sim);
	    sim->cumulative_inter_arrival_time += duration;
	    sim->number_of_users++;
	    Event e = event_init(sim->current + duration, have_space_in_waiting_line(sim) ? ARRIVAL : LOST);
	    schedule_event(sim, e);
    }
}
/**
 * Description: generate the departure of the event from the system
 */
void generate_departure(Queue_Simulator sim, Event event) {
	return_if_null("sim", sim,);
	return_if_null("event", event,);
	static double duration;
	duration = server_getServiceTime(sim->server);
	event_setType(event, DEPARTURE);
	event_setScheduled(event, sim->current + duration);
	schedule_event(sim, event);
	sim->busy_servers++;
	sim->cumulative_waiting_time += sim->current - event_getArrival(event);
	sim->cumulative_service_time += duration;
}
/**
 * Description: manage how the system work in case of an arrival event
 */
void manage_arrival(Queue_Simulator sim, Event event) {
	return_if_null("sim", sim,);
	return_if_null("event", event,);
	generate_new_arrival(sim);
	if(sim->total_users==0) {
		sim->cumulative_free_system_time+=sim->current-sim->last_event_time;
	}
	sim->total_users++;
	if (sim->busy_servers < sim->number_of_servers) {
		generate_departure(sim, event);
	} else {
		eventlist_insert(sim->waiting_events, event);
	}
}
/**
 * Description: manage how the system work in case of a departure event
 */
void manage_departure(Queue_Simulator sim, Event event) {
	return_if_null("sim", sim,);
	return_if_null("event", event,);
	sim->busy_servers--;
	sim->total_users--;
	sim->number_of_services++;
	if (!eventlist_is_empty(sim->waiting_events)) {
		generate_departure(sim, eventlist_extract(sim->waiting_events));
	}
	event_free(event);
}
/**
 * Description: manage how the system work in case of a lost event
 */
void manage_lost(Queue_Simulator sim, Event event) {
	return_if_null("sim", sim,);
	return_if_null("event", event,);
	generate_new_arrival(sim);
	sim->number_of_losses++;
}
/**
 * Description: start the simulation with the parameters set
 */
void queue_simulator_run(Queue_Simulator sim) {
	static Event event;
	return_if_null("sim", sim,);
	generate_new_arrival(sim);
//	double last_percentage = 0.0;
//	double percentage_step = 0.1 / 100;
	while (sim->current < sim->maximum && have_schedulable_events(sim)) {
//		if (sim->current / sim->maximum - last_percentage > percentage_step) {
//			last_percentage = sim->current / sim->maximum;
//			fprintf(stderr, "%5.1lf\n", last_percentage * 100);
//		}
		event = eventlist_extract(sim->scheduled_events);
		sim->last_event_time = sim->current;
		sim->current = event_getScheduled(event);
		if (event_getType(event) == ARRIVAL) {
			manage_arrival(sim, event);
		} else if (event_getType(event) == DEPARTURE) {
			manage_departure(sim, event);
		} else if (event_getType(event) == LOST) {
		    manage_lost(sim, event);
		}
		else {
			error_reporting("Unknoun error");
		}
	}
}
/**
 * Description: print on the stream the input of the simulation
 */
void queue_simulator_inputs(FILE *stream, Queue_Simulator sim) {
	return_if_null("sim", sim,);
	if (stream == NULL || stream == stdin) {
		stream = stdout;
	}
	fprintf(stream, "INPUT\n");
	fprintf(stream, "number of servers = %ld\n", sim->number_of_servers);
	fprintf(stream, "lambda = %lf\n", sim->lambda);
	server_print(stream, sim->server);
	fprintf(stream, "maximum = %lf\n", sim->maximum);
	fprintf(stream, "population_size = %ld\n", sim->population_size);
}
/**
 * Description: print on the stream the simulated results of the simulation
 */
void prints_simulated_results(FILE *stream, Queue_Simulator sim) {
	return_if_null("sim", sim,);
	if (stream == NULL || stream == stdin) {
		stream = stdout;
	}
	fprintf(stream, "Simulated rho = lambda / (#s * mu) = %.6lf\n", simulated_rho(sim));
	fprintf(stream, "Simulated number of losses = %ld\n", sim->number_of_losses);
	fprintf(stream, "Simulated average inter arrival time = %.6lf\n", simulated_average_inter_arrival_time(sim));
	fprintf(stream, "Simulated average service time = %.6lf\n", simulated_average_service_time(sim));
	fprintf(stream, "Simulated average waiting time = %.6lf\n", simulated_average_waiting_time(sim));
	fprintf(stream, "Simulated average response (service+waiting) time = %.6lf\n", simulated_average_response_time(sim));
	fprintf(stream, "Simulated average number of customers = %.6lf\n", simulated_average_number_of_users(sim));
	fprintf(stream, "Simulated idle system probability = %.6lf\n", simulated_probability_of_idle_system(sim));
	fprintf(stream, "Simulated loss probability = %.6lf\n", simulated_loss_probability(sim));
	fprintf(stream, "Simulated average of busy servers = %.6lf\n", simulated_average_busy_servers(sim));
}
/**
 * Description: print on the stream the theoretic results for the simulation
 */
void prints_theoretic_results(FILE *stream, Queue_Simulator sim) {
	return_if_null("sim", sim,);
	if (stream == NULL || stream == stdin) {
		stream = stdout;
	}
	fprintf(stream, "Theoretic rho = lambda / (#s * mu) = %.6lf\n", theoretic_rho(sim));
	fprintf(stream, "Theoretic average inter arrival time = %.6lf\n", theoretic_average_inter_arrival_time(sim));
	fprintf(stream, "Theoretic average service time = %.6lf\n", theoretic_average_service_time(sim));
	if(theoretic_average_waiting_time(sim)>=0) { fprintf(stream, "Theoretic average waiting time = %.6lf\n", theoretic_average_waiting_time(sim)); }
	if(theoretic_average_response_time(sim)>=0) { fprintf(stream, "Theoretic average response (service+waiting) time = %.6lf\n", theoretic_average_response_time(sim)); }
	if(theoretic_average_number_of_users(sim)>=0) { fprintf(stream, "Theoretic average number of customers = %.6lf\n", theoretic_average_number_of_users(sim)); }
	if(theoretic_probability_of_idle_system(sim)>=0) { fprintf(stream, "Theoretic idle system probability = %.6lf\n", theoretic_probability_of_idle_system(sim)); }
	if(theoretic_loss_probability(sim)>=0) { fprintf(stream, "Theoretic loss probability = %.6lf\n", theoretic_loss_probability(sim)); }
	fprintf(stream, "Theoretic average of busy servers = %.6lf\n", theoretic_average_busy_servers(sim));
}
#define evaluate_relative_error(theoric, simulated) (theoric==simulated ? 0 : 100*ABS(1-(simulated)/(theoric)))
/**
 * Description: print on the stream the percentage of error beetwen simulated and theoretic results
 */
void prints_percentage_of_error_results(FILE *stream, Queue_Simulator sim) {
	return_if_null("sim", sim,);
	if (stream == NULL || stream == stdin) {
		stream = stdout;
	}
	fprintf(stream, "Error on rho = lambda / (#s * mu) = %.6lf%%\n", evaluate_relative_error(theoretic_rho(sim), simulated_rho(sim)));
	fprintf(stream, "Error on average inter arrival time = %.6lf%%\n", evaluate_relative_error(theoretic_average_inter_arrival_time(sim), simulated_average_inter_arrival_time(sim)));
	fprintf(stream, "Error on average service time = %.6lf%%\n", evaluate_relative_error(theoretic_average_service_time(sim), simulated_average_service_time(sim)));
	if(theoretic_average_waiting_time(sim)>=0) { fprintf(stream, "Error on average waiting time = %.6lf%%\n", evaluate_relative_error(theoretic_average_waiting_time(sim), simulated_average_waiting_time(sim))); }
	if(theoretic_average_response_time(sim)>=0) { fprintf(stream, "Error on average response (service+waiting) time = %.6lf%%\n", evaluate_relative_error(theoretic_average_response_time(sim), simulated_average_response_time(sim))); }
	if(theoretic_average_number_of_users(sim)>=0) { fprintf(stream, "Error on average number of customers = %.6lf%%\n", evaluate_relative_error(theoretic_average_number_of_users(sim), simulated_average_number_of_users(sim))); }
	if(theoretic_probability_of_idle_system(sim)>=0) { fprintf(stream, "Error on idle system probability = %.6lf%%\n", evaluate_relative_error(theoretic_probability_of_idle_system(sim), simulated_probability_of_idle_system(sim))); }
	if(theoretic_loss_probability(sim)>=0) { fprintf(stream, "Error on loss probability = %.6lf%%\n", evaluate_relative_error(theoretic_loss_probability(sim), simulated_loss_probability(sim))); }
	fprintf(stream, "Error on average of busy servers = %.6lf%%\n", evaluate_relative_error(theoretic_average_busy_servers(sim), simulated_average_busy_servers(sim)));
}
/**
 * Description: print on the stream the results of the simulation
 */
void queue_simulator_outputs(FILE *stream, Queue_Simulator sim) {
	return_if_null("sim", sim,);
	if (stream == NULL || stream == stdin) {
		stream = stdout;
	}
	fprintf(stream, "RESULTS\n");
	fprintf(stream, "Time of end simulation = %.6lf\n", sim->current);
	fprintf(stream, "Total number of users generated = %ld\n", sim->number_of_users);
	fprintf(stream, "Total number of users served = %ld\n", sim->number_of_services);
	fprintf(stream, "\n");
	prints_simulated_results(stream, sim);
	fprintf(stream, "\n");
	prints_theoretic_results(stream, sim);
	fprintf(stream, "\n");
	prints_percentage_of_error_results(stream, sim);
}
/**
 * Description: release all the memory required from the simulator
 */
void queue_simulator_free(Queue_Simulator sim) {
	return_if_null("sim", sim,);
	eventlist_free(sim->scheduled_events);
	eventlist_free(sim->waiting_events);
	event_free_memory();
	free(sim);
}

