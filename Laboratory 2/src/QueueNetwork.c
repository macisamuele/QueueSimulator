#include "QueueNetwork.h"
#include "Queue.h"
#include "event.h"
#include "server.h"
#include "simulation_time.h"
#include "random_numbers.h"

struct QueueNetwork_t {	//series of N queues
//input parameters
	double lambda;	//arrival rate
	Time maximum;	//maximum time to simulate
	unsigned long number_of_queues; //number of queues in the system
	Server *servers;	//description of the servers of the system
	unsigned long *number_of_servers;//information about the number of servers of the system
	int present_initial_seed;   //flag that say if the initial seed was choosed from the user
	long initial_seed;  //initial seed used
//internal variables
	Time current_time;	//current simulation time
	EventList scheduled_events;	//list of events to be servers (only one list for all the system, the event know the right queue)
	Queue *queues;	//array of queues
	unsigned long busy_queues;	//number of queues currently busy
	unsigned long total_users;	//number of users currently in the system
	Time last_event_time;	//time in which occur the last event
//measures
	unsigned long number_of_users;	//number of users entered in the queue
	unsigned long number_of_services;	//number of ended services
	Time cumulative_inter_arrival_time;	//summation of all the inter arrival time
	Time cumulative_response_time;//summation of all the respons time of all the events putted in service
};

void queuenetwork_manage_arrival(QueueNetwork queuenetwork);
void queuenetwork_manage_departure(QueueNetwork queuenetwork, Event event);

/**
 * Description: allocate the network of serial queues
 * Inputs:
 * 			lambda = arrival rate in the system
 * 			maximum = maximum time of the simulations
 * 			number_of_queues = number of queues of the system (serial configuration of them)
 * 			*servers = servers[i] contains the information about the kind of server of the (i)-th queue of the network
 * 			*number_of_servers = number_of_servers[i] contains the information about the number of servers of the (i)-th queue of the network
 * 			present_initial_seed = 0 if you do not want to force an initial seed, 1 otherwise
 * 			initial_seed must be the initial seed if present_initial_seed is set to 1 otherwise any value can be putted (it will not be used)
 * Return: the new network
 */
QueueNetwork queuenetwork_init(double lambda, Time maximum, unsigned long number_of_queues, Server *servers, unsigned long *number_of_servers, int present_initial_seed, long initial_seed) {
	static unsigned long i;
	QueueNetwork queuenetwork;
	_alloc(queuenetwork, QueueNetwork, 1);
	queuenetwork->lambda = lambda;
	queuenetwork->maximum = maximum;
	queuenetwork->number_of_queues = number_of_queues;
	queuenetwork->servers = servers;
	queuenetwork->number_of_servers = number_of_servers;
	queuenetwork->current_time = 0.0;
	queuenetwork->scheduled_events = eventlist_init();
	_alloc(queuenetwork->queues, Queue *, number_of_queues);
	for (i = 0; i < number_of_queues; i++) {
		queuenetwork->queues[i] = queue_init(number_of_servers[i], servers[i], &queuenetwork->current_time, &queuenetwork->scheduled_events);
	}
	queuenetwork->busy_queues = 0;
	queuenetwork->total_users = 0;
	queuenetwork->last_event_time = 0.0;
	queuenetwork->number_of_users = 0;
	queuenetwork->number_of_services = 0;
	queuenetwork->cumulative_inter_arrival_time = 0.0;
	queuenetwork->cumulative_response_time = 0.0;
    queuenetwork->present_initial_seed = present_initial_seed;
    if(present_initial_seed) {
        queuenetwork->initial_seed = initial_seed;
        set_seed(initial_seed);
    }
	return queuenetwork;
}
/**
 * Description: release all the memory required from the queuenetwork
 */
void queuenetwork_free(QueueNetwork queuenetwork) {
	static unsigned long i;
	return_if_null("queuenetwork", queuenetwork,);
	for (i = 0; i < queuenetwork->number_of_queues; i++) {
		server_free(queuenetwork->servers[i]);
	}
	free(queuenetwork->number_of_servers);
	eventlist_free(queuenetwork->scheduled_events);
	for (i = 0; i < queuenetwork->number_of_queues; i++) {
		queue_free(queuenetwork->queues[i]);
	}
	free(queuenetwork);
}

#define have_schedulable_events(queuenetwork) (!eventlist_is_empty(queuenetwork->scheduled_events))
#define get_scheduled_event(queuenetwork) (eventlist_extract(queuenetwork->scheduled_events))
#define schedule_event(queuenetwork, event) eventlist_insert(queuenetwork->scheduled_events, event)

void queuenetwork_run(QueueNetwork queuenetwork) {
	static Event event;
	return_if_null("queuenetwork", queuenetwork,);
	queuenetwork_manage_arrival(queuenetwork);
	while (queuenetwork->current_time < queuenetwork->maximum) {
		event = get_scheduled_event(queuenetwork);
		queuenetwork->last_event_time = queuenetwork->current_time;
		queuenetwork->current_time = event_getScheduled(event);
		if (event_getType(event) == ARRIVAL && event_getQueueID(event) == 0) {
			queuenetwork_manage_arrival(queuenetwork);
		} else if (event_getType(event) == DEPARTURE && event_getQueueID(event) == (queuenetwork->number_of_queues - 1)) {
			queuenetwork_manage_departure(queuenetwork, event);
		}
		event = queue_manage_event(queuenetwork->queues[event_getQueueID(event)], event);
		if (event == NULL) {
			error_reporting("Unknoun error");
		} else {
			if (event_getType(event) == EXIT) {
				if (event_getQueueID(event) == queuenetwork->number_of_queues - 1) {
					queuenetwork->cumulative_response_time += queuenetwork->current_time - event_getSystemArrival(event);
					event_free(event);
				} else {
					event_setType(event, ARRIVAL);
					event_setQueueID(event, event_getQueueID(event) + 1);
					schedule_event(queuenetwork, event);
				}
			}
		}
	}
}

#define inter_arrival_time(queuenetwork) negexp(1.0/queuenetwork->lambda)

void queuenetwork_manage_arrival(QueueNetwork queuenetwork) {
	static double duration;
	return_if_null("queuenetwork", queuenetwork,);
	duration = inter_arrival_time(queuenetwork);
	schedule_event(queuenetwork, event_init(queuenetwork->current_time + duration, ARRIVAL, 0));
	queuenetwork->busy_queues++;
	queuenetwork->cumulative_inter_arrival_time += duration;
	queuenetwork->number_of_users++;
	queuenetwork->total_users++;
}
void queuenetwork_manage_departure(QueueNetwork queuenetwork, Event event) {
	return_if_null("queuenetwork", queuenetwork,);
	return_if_null("event", event,);
	queuenetwork->busy_queues--;
	queuenetwork->total_users--;
	queuenetwork->number_of_services++;
}
QueueNetwork queuenetwork_init_from_command_line(int argc, char *argv[]) {
	static int error, offset, n, present_initial_seed;
	static double lambda, max_time;
	static unsigned long number_of_queues, queue, *number_of_servers;
	static long initial_seed;
	static Server *servers;
	number_of_servers = NULL;
	servers = NULL;
	present_initial_seed = 0;
	error = 0;
	if(argc>3) {   //DA METTERE I CONTROLLI SUGLI INDICI!!!
        if(!sscanf(argv[0], "%lf", &lambda)) { error=1; }
        if(!sscanf(argv[1], "%ld", &number_of_queues)) { error=1; }
        if((number_of_servers = (unsigned long *)malloc(number_of_queues*sizeof(unsigned long)))==NULL) { error=1; }
        if((servers = (Server *)calloc(number_of_queues, sizeof(Server)))==NULL) { error=1; }
        offset = 0;
        for(queue=0; queue<number_of_queues && !error; queue++) {
        	if(argc>2+offset+1) {
				if(!sscanf(argv[2+offset], "%ld", &number_of_servers[queue])) { error=1; }
				if((servers[queue] = server_command_line_init(argc-2-1-offset, argv+2+1+offset, &n)) == NULL) { error=1; }
				offset+=n+1;
        	}
        	else {
        		error=1;
        	}
        }
        if(!error) {
			if(!sscanf(argv[2+offset], "%lf", &max_time)) { error=1; }
			if(argc>2+offset+1) {
				if(!sscanf(argv[2+offset+1], "%ld", &initial_seed)) { error=1; }
				else { present_initial_seed=1; }
			}
        }
	}
	else {
		error = 1;
	}
	if(error) {
		if(number_of_servers!=NULL) { free(number_of_servers); }
		if(servers!=NULL) { for(queue=0; queue<number_of_queues && !error; queue++) { server_free(servers[queue]); } free(servers); }

		fprintf(stderr, "Wrong number of command line paramenters.\n\tUsage: command lambda number_of_queues <queue1> ... <queue_n> maximum_simulation_time\n");
		fprintf(stderr, "<queue_i> is the i-th queue of the chain: number_of_servers <server>\n");
        fprintf(stderr, "%s\n\n", server_getUsage());
        warning_reporting("Wrong input line parameters. The initialization continue through the keyboard.");
		return queuenetwork_keyboard_init();
	}
	return queuenetwork_init(lambda, max_time, number_of_queues, servers, number_of_servers, present_initial_seed, initial_seed);
}
QueueNetwork queuenetwork_keyboard_init() {
    static int present_initial_seed;
    static double lambda, max_time;
	static unsigned long number_of_queues, *number_of_servers, queue;
	static Server *servers;
	static long initial_seed;
	static QueueNetwork queuenetwork;
	number_of_servers = NULL;
	servers = NULL;
	printf("Insert the arrival rate for the network, lambda: ");
    scanf("%lf", &lambda);
    printf("Insert the number of queues putted in series: ");
    scanf("%ld", &number_of_queues);
    if((number_of_servers = (unsigned long *)malloc(number_of_queues*sizeof(unsigned long)))==NULL) {
        return NULL;
    }
    if((servers = (Server *)calloc(number_of_queues, sizeof(Server)))==NULL) {
        free(number_of_servers);
        return NULL;
    }
    for(queue=0; queue<number_of_queues; queue++) {
        printf("Insert the number of servers of the queue n. %ld: ", queue);
	    scanf("%ld", &number_of_servers[queue]);
        printf("Server input of queue n. %ld\n", queue);
		if((servers[queue] = server_keyboard_init()) == NULL) {
		    for(queue--; queue>0; queue--) {
		        server_free(servers[queue]);
		    }
	        server_free(servers[queue]);
		    free(servers);
		    free(number_of_servers);
		    return NULL;
		}
    }
    printf("Insert the maximum simulation time: ");
	scanf("%lf", &max_time);
	printf("Is present an initial seed? Put 0 to say NO, YES otherwise: ");
	scanf("%d", &present_initial_seed);
	if(present_initial_seed!=0) {
        present_initial_seed=1;
        printf("Insert the initial seed: ");
        scanf("%ld", &initial_seed);
	}
	if((queuenetwork = queuenetwork_init(lambda, max_time, number_of_queues, servers, number_of_servers, present_initial_seed, initial_seed))==NULL) {
	    for(queue=0; queue<number_of_queues; queue++) {
	        server_free(servers[queue]);
	    }
	    free(servers);
	    free(number_of_servers);
	    return NULL;
	}
	return queuenetwork;
}

#define simulated_average_inter_arrival_time(queuenetwork) (queuenetwork->cumulative_inter_arrival_time / queuenetwork->number_of_users)
#define simulated_average_response_time(queuenetwork) (queuenetwork->cumulative_response_time / (queuenetwork->number_of_services + queuenetwork->busy_queues))  //0 DEVE ESSERE SISTEMATO ... DEVO TENER IN CONTO DEL NUMERO DEGLI UTENTI IN SERVIZIO!
#define simulated_average_number_of_users(queuenetwork) (simulated_average_response_time(queuenetwork)*queuenetwork->lambda)
void prints_simulated_results(FILE *stream, QueueNetwork queuenetwork) {
    static unsigned long i;
	return_if_null("queuenetwork", queuenetwork,);
	if (stream == NULL || stream == stdin) {
		stream = stdout;
	}
	fprintf(stream, "\nQUEUE NETWORK - SIMULATED RESULTS\n");
	fprintf(stream, "QN | Simulated average inter arrival time = %.20lf\n", simulated_average_inter_arrival_time(queuenetwork));
	fprintf(stream, "QN | Simulated average response (services+waiting) time = %.20lf\n", simulated_average_response_time(queuenetwork));
	fprintf(stream, "QN | Simulated average number of customers = %.20lf\n", simulated_average_number_of_users(queuenetwork));
	for(i=0; i<queuenetwork->number_of_queues; i++) {
    	fprintf(stream, "QUEUE %ld - SIMULATED RESULTS\n", i);
 	    fprintf(stream, "Q%ld | Simulated average inter arrival time = %.20lf\n", i, queue_getAverageInterArrivalTime(queuenetwork->queues[i]));
 	    fprintf(stream, "Q%ld | Simulated average response (services+waiting) time = %.20lf\n", i, queue_getAverageResponseTime(queuenetwork->queues[i]));
 	    fprintf(stream, "Q%ld | Simulated average number of customers = %.20lf\n", i, queue_getAverageNumberOfCustomers(queuenetwork->queues[i]));
 	    fprintf(stream, "Q%ld | Simulated offered load, Rho = %.20lf\n", i, queue_getRho(queuenetwork->queues[i]));
	}
}
#define theoretic_average_inter_arrival_time(queuenetwork) (queuenetwork->lambda)
double theoretic_average_response_time(QueueNetwork queuenetwork) {
	static unsigned long i;
	static double response;
	return_if_null("queuenetwork", queuenetwork, -1);
	response = 0;
	for(i=0; i<queuenetwork->number_of_queues; i++) {
		response += queue_getTheoreticAverageResponseTime(queuenetwork->queues[i], queuenetwork->lambda);
	}
	return response;
}
#define theoretic_average_number_of_users(queuenetwork) (theoretic_average_response_time(queuenetwork)*queuenetwork->lambda)
void prints_theoretic_results(FILE *stream, QueueNetwork queuenetwork) {
    static unsigned long i;
	return_if_null("queuenetwork", queuenetwork,);
	if (stream == NULL || stream == stdin) {
		stream = stdout;
	}
	fprintf(stream, "\nQUEUE NETWORK - THEORETIC RESULTS\n");
	if(theoretic_average_inter_arrival_time(queuenetwork)>=0) { fprintf(stream, "QN | Theoretic average inter arrival time = %.20lf\n", theoretic_average_inter_arrival_time(queuenetwork)); }
	if(theoretic_average_response_time(queuenetwork)>=0) {fprintf(stream, "QN | Theoretic average response (services+waiting) time = %.20lf\n", theoretic_average_response_time(queuenetwork)); }
	if(theoretic_average_number_of_users(queuenetwork)>=0) { fprintf(stream, "QN | Theoretic average number of customers = %.20lf\n", theoretic_average_number_of_users(queuenetwork)); }
	for(i=0; i<queuenetwork->number_of_queues; i++) {
    	fprintf(stream, "QUEUE %ld - THEORETIC RESULTS\n", i);
   	    if(queue_getTheoreticAverageResponseTime(queuenetwork->queues[i], queuenetwork->lambda)>=0) { fprintf(stream, "Q%ld | Theoretic average response (services+waiting) time = %.20lf\n", i, queue_getTheoreticAverageResponseTime(queuenetwork->queues[i], queuenetwork->lambda)); }
 	    if(queue_getTheoreticAverageNumberOfCustomers(queuenetwork->queues[i], queuenetwork->lambda)>=0) { fprintf(stream, "Q%ld | Theoretic average number of customers = %.20lf\n", i, queue_getTheoreticAverageNumberOfCustomers(queuenetwork->queues[i], queuenetwork->lambda)); }
 	    if(queue_getTheoreticRho(queuenetwork->queues[i], queuenetwork->lambda)>=0) { fprintf(stream, "Q%ld | Theoretic offered load, Rho = %.20lf\n", i, queue_getTheoreticRho(queuenetwork->queues[i], queuenetwork->lambda)); }
	}
}
#define ABS(a) ((a)>0 ? (a) : (a)*(-1))
#define evaluate_relative_error(theoric, simulated) (theoric==simulated ? 0 : 100*ABS(1-(simulated)/(theoric)))
void prints_percentage_of_error_results(FILE *stream, QueueNetwork queuenetwork) {
	static unsigned long i;
	return_if_null("queuenetwork", queuenetwork,);
	if (stream == NULL || stream == stdin) {
		stream = stdout;
	}
	fprintf(stream, "\nQUEUE NETWORK - RELATIVE ERRORS ON RESULTS\n");
	if(theoretic_average_inter_arrival_time(queuenetwork)>=0) { fprintf(stream, "QN | Error on average inter arrival time = %.6lf%%\n", evaluate_relative_error(theoretic_average_inter_arrival_time(queuenetwork), simulated_average_inter_arrival_time(queuenetwork))); }
	if(theoretic_average_response_time(queuenetwork)>=0) {fprintf(stream, "QN | Error on average response (services+waiting) time = %.6lf%%\n",
	evaluate_relative_error(theoretic_average_response_time(queuenetwork), simulated_average_response_time(queuenetwork))); }
	if(theoretic_average_number_of_users(queuenetwork)>=0) { fprintf(stream, "QN | Error on average number of customers = %.6lf%%\n",
			evaluate_relative_error(theoretic_average_number_of_users(queuenetwork), simulated_average_number_of_users(queuenetwork))); }
	for(i=0; i<queuenetwork->number_of_queues; i++) {
    	fprintf(stream, "QUEUE %ld - RELATIVE ERRORS ON RESULTS\n", i);
    	if(queue_getTheoreticAverageResponseTime(queuenetwork->queues[i], queuenetwork->lambda)>=0) {
    		fprintf(stream, "Q%ld | Error on average response (services+waiting) time = %.6lf%%\n", i,  evaluate_relative_error(queue_getTheoreticAverageResponseTime(queuenetwork->queues[i], queuenetwork->lambda), queue_getAverageResponseTime(queuenetwork->queues[i])));
    	}
    	if(queue_getTheoreticAverageNumberOfCustomers(queuenetwork->queues[i], queuenetwork->lambda)>=0) {
    		fprintf(stream, "Q%ld | Error on average number of customers = %.6lf%%\n", i, evaluate_relative_error(queue_getTheoreticAverageNumberOfCustomers(queuenetwork->queues[i], queuenetwork->lambda), queue_getAverageNumberOfCustomers(queuenetwork->queues[i])));
    	}
 	    if(queue_getTheoreticRho(queuenetwork->queues[i], queuenetwork->lambda)>=0) {
    		fprintf(stream, "Q%ld | Error on offered load, Rho = %.6lf%%\n", i, evaluate_relative_error(queue_getTheoreticRho(queuenetwork->queues[i], queuenetwork->lambda), queue_getRho(queuenetwork->queues[i])));
 	    }
	}
}
void queuenetwork_outputs(FILE *stream, QueueNetwork queuenetwork) {
    static unsigned long i;
	return_if_null("queuenetwork", queuenetwork,);
	if (stream == NULL || stream == stdin) {
		stream = stdout;
	}
	fprintf(stream, "\nQUEUE NETWORK - RESULTS\n");
	fprintf(stream, "QN | Time of end simulation = %.20lf\n", queuenetwork->current_time);
	fprintf(stream, "QN | Total number of users generated = %ld\n", queuenetwork->number_of_users);
	fprintf(stream, "QN | Total number of users served = %ld\n", queuenetwork->number_of_services);
	for(i=0; i<queuenetwork->number_of_queues; i++) {
	    fprintf(stream, "QUEUE %ld - RESULTS\n", i);
	    fprintf(stream, "Q%ld | Total number of users generated = %ld\n", i, queue_getNumberOfUsersGenerated(queuenetwork->queues[i]));
	    fprintf(stream, "Q%ld | Total number of users served = %ld\n", i, queue_getNumberOfServices(queuenetwork->queues[i]));
	}
	prints_simulated_results(stream, queuenetwork);
	prints_theoretic_results(stream, queuenetwork);
	prints_percentage_of_error_results(stream, queuenetwork);
	fprintf(stream, "\nRANDOM GENERATION INFORMATIONS\n");
	if(queuenetwork->present_initial_seed) {
	    fprintf(stream, "First seed: %ld\n", queuenetwork->initial_seed);
	}
	fprintf(stream, "Last seed: %ld\n", get_seed());
}
void queuenetwork_inputs(FILE *stream, QueueNetwork queuenetwork) {
	static unsigned long i;
	return_if_null("queuenetwork", queuenetwork,);
	if (stream == NULL || stream == stdin) {
		stream = stdout;
	}
	fprintf(stream, "\nQUEUE NETWORK - INPUTS\n");
	fprintf(stream, "QN | Arrival Rate, lambda = %.20lf\n", queuenetwork->lambda);
	fprintf(stream, "QN | Number of Queues = %ld\n", queuenetwork->number_of_queues);
	fprintf(stream, "QN | Maximum Simulation Time = %.20lf\n", queuenetwork->maximum);
	for(i=0; i<queuenetwork->number_of_queues; i++) {
		fprintf(stream, "QUEUE %ld - INPUTS\n", i);
		fprintf(stream, "Q%ld | Server: ", i);
		server_print(stream, queuenetwork->servers[i]);
		fprintf(stream, "Q%ld | Number of Servers: %ld\n", i, queuenetwork->number_of_servers[i]);
	}
}
