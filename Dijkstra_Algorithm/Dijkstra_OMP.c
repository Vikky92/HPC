
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <omp.h>

#include "Dijkstra_tools.h"

long N;
int Init;

void inputParse(int argc, char** argv); //Checks for the right argument at the input
void printUsage(int argc, char** argv); //This is used to print the results of the 
void dijkstra_run(graph* G, long initial_node, char debug); // the parallel dijkstra's algorithm implementation 
void SchedulerTest(int numThreads, graph* G); //the tests for the four schedulers used 

int main(int argc, char *argv[])
{
	long i;
	graph G;
	inputParse(argc, argv); //parsing of the input from the command line

	if(N == 1)
	{
		GraphTestGenerate(&G); //Generate default sized input matrix
	} 
	
	else 
	{
		GraphGenerate(N, Init, &G, 0); //Generate N*N sized input matrix
	}


	// This is used to call the parallel dijkstra's algorithm and spawning the threads using the omp_set_num_threads

	printf("\nTesting with max 2 Threads\n");
	SchedulerTest(2,&G);

	printf("\nTesting with max 4 Threads\n");
	SchedulerTest(4,&G);

	printf("\nTesting with max 8 Threads\n");
	SchedulerTest(8,&G);

	printf("\nTesting with max 8 Threads\n");
	SchedulerTest(16,&G);

	printf("\nTesting with max 8 Threads\n");
	SchedulerTest(32,&G);

	printf("\nTesting with max 8 Threads\n");
	SchedulerTest(64,&G);

	printf("\nTesting with max 8 Threads\n");
	SchedulerTest(128,&G);

	printf("\nTesting with max 8 Threads\n");
	SchedulerTest(256,&G);

	
	return EXIT_SUCCESS;
}

void SchedulerTest(int numThreads, graph* G)
{
	double runtime;
	//Set max numThreads
	omp_set_num_threads(numThreads); //Sets the maximum no. of processing elements 
	
	//	omp_set_schedule(omp_sched_*, G->N/100) using this to set scheduler and use numberNodes/100 chunks for each thread


	//STATIC SCHEDULER IMPLEMENTATION
	printf("Scheduler (Static, %d)", G->N/100 );
	resetGraph(G);
	omp_set_schedule(omp_sched_static, G->N/100);  
	tick();		//Starts the Timer
		dijkstra_run(G, 0, 0);
	runtime = tack(); //Stops the Timer 
	printf("working for [%f] sec.\n",runtime);

	//DYNAMIC SCHEDULER IMPLEMENTATION
	printf("Scheduler (dynamic, %d)", G->N/100 ); 
	resetGraph(G);
	omp_set_schedule(omp_sched_dynamic, G->N/100); 
	tick(); //Starts the timer
		dijkstra_run(G, 0, 0); 
	runtime = tack(); //Stops the timer
	printf("working for [%f] sec.\n",runtime);

	//GUIDED SCHEDULER IMPLEMENTATION
	printf("Scheduler (guided, %d)", G->N/100 );
	resetGraph(G);
	omp_set_schedule(omp_sched_guided, G->N/100);
	tick(); //Starts the timer
		dijkstra_run(G, 0, 0);
	runtime = tack(); //Stops the timer
	printf("working for [%f] sec.\n",runtime);

	//AUTO SCHEDULER IMPLEMENTATION
	printf("Scheduler (auto, %d)", G->N/100 );
	resetGraph(G);
	omp_set_schedule(omp_sched_auto, G->N/100);
	tick(); //Starts the timer
		dijkstra_run(G, 0, 0);
	runtime = tack(); //Stops the timer
	printf("working for [%f] sec.\n",runtime);
}

void dijkstra_run(graph* G, long initial_node, char debug)
{
	long i,j,k;
	long aN; //actualNode
	G->D[initial_node] = 0;
	aN = initial_node;

	if(debug)
	{
		printf("Running Dijkstra on graph\n");
		printGraph(G);
	}

	for(i = 0; i < G->N; i++)
	{
		G->visited[aN] = VISITED;

		if(debug)
		{
			printf("It[%d] aN [%d]",i, aN); printStatus(G); printf("\n");
		}

		//Find all nodes connected to aN
		#pragma omp parallel for schedule(runtime)
		for(j=0; j< G->N; j++)
		{
			if( (G->node[aN][j] != NO_CONN) )
			{
				if( (G->D[aN] + G->node[aN][j]) < G->D[j] )
				{
					G->D[j] = (G->D[aN] + G->node[aN][j]);
				}
			}
		}

		aN = NextNode(G);
	}
	//printf("Finished Dijkstra\n");
}

void printUsage(int argc, char** argv){
	printf("Usage: %s NUMER_OF_POINTS [SRAND_INIT_VALUE]\n",argv[0]);
}

void inputParse(int argc, char** argv)
{
	if ( argc < 2){
		printUsage(argc, argv);
		exit(EXIT_FAILURE);
	}

	errno = 0; // To distinguish success/failure after call
	N = atol(argv[1]);

	/* Check for various possible errors */

	if ((errno == ERANGE && (N == LONG_MAX || N == LONG_MIN)) || (errno != 0 && N == 0)) {
	   perror("atol");
	   exit(EXIT_FAILURE);
	}

	if(N <= 0){
		printf("Invalid number of points! Number of points must be bigger than zero.\n");
		exit(EXIT_FAILURE);
	}

	//Check for second argument being the random init value
	if(argc == 3)
	{
		errno = 0; // To distinguish success/failure after call
		Init = atol(argv[2]);

		/* Check for various possible errors */

		if ((errno == ERANGE && (Init == LONG_MAX || Init == LONG_MIN)) || (errno != 0 && Init == 0)) {
		   perror("atol");
		   exit(EXIT_FAILURE);
		}
	} else {
		Init = -1;
	}

	if(Init < 0){
		time_t t1;
		struct tm* t2 = NULL;
		int sec;
		t1 = time( NULL );
		t2 = gmtime(&t1);
		sec = t2->tm_sec;
		Init = sec;
	}
}
