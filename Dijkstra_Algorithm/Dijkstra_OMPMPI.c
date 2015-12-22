
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <omp.h>
#include "mpi.h"

#include "Dijkstra_tools.h"

long N;
int Init;

int size, rank;

void InputParse(int argc, char** argv);
void printUsage(int argc, char** argv);
void SchedulerTest(int numThreads, graph* G, char debug);
void dijkstra(graph* G, long initial_node, char debug);
void par_dijkstra(graph* G, long initial_node, long nStart, long nEnd, char debug);

int main(int argc, char **argv)
{
	long i;
	graph G;
	char debugFlag = 0;

	debugFlag=0;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);        /* get current process id */
	MPI_Comm_size (MPI_COMM_WORLD, &size);        /* get number of processes */

	InputParse(argc, argv);

	if(rank == 0)
	{
		if(N == 1){
			GraphTestGenerate(&G);
		} else {
			GraphGenerate(N, Init, &G, debugFlag);
		}
	} else {
		if(N == 1)
			N = 6;
		GraphEmptyGenerate(N, &G);
	}
	N = G.N;

	if(debugFlag){
		enableDebug(N);
	}

	if((debugFlag == 1) && (rank == 0) ){
		printf("Using graph\n");
		printGraph(&G);
	}

	if(rank == 0) printf("\nTesting with max 16 Threads\n");
	SchedulerTest(2,&G, debugFlag);

	if(rank== 0) printf("\nTesting with max 32 Threads\n");
	SchedulerTest(4,&G, debugFlag);

	if(rank == 0) printf("\nTesting with max 64 Threads\n");
	SchedulerTest(8,&G, debugFlag);

	MPI_Finalize();
	return EXIT_SUCCESS;
}

void SchedulerTest(int numThreads, graph* G, char debug)
{
	double runtime;

	//This is used to set the number of threads in the OPENMPI implementation to 2,4 or 8. 
	omp_set_num_threads(numThreads);

	if(rank== 0) printf("Scheduler (Static, %d)", G->N/100 );
	resetGraph(G);
	omp_set_schedule(omp_sched_static, G->N/100);
	if(rank== 0) tick();
		dijkstra(G, 0, debug);
	if(rank== 0){
		runtime = tack();
		printf("working for [%f] sec.\n",runtime);
	}


	if(rank== 0) printf("Scheduler (dynamic, %d)", G->N/100 );
	resetGraph(G);
	omp_set_schedule(omp_sched_dynamic, G->N/100);
	if(rank== 0) tick();
		dijkstra(G, 0, debug);
	if(rank== 0){
		runtime = tack();
		printf("working for [%f] sec.\n",runtime);
	}

	if(rank== 0) printf("Scheduler (guided, %d)", G->N/100 );
	resetGraph(G);
	omp_set_schedule(omp_sched_guided, G->N/100);
	if(rank == 0) tick();
		dijkstra(G, 0, debug);
	if(rank == 0){
		runtime = tack();
		printf("working for [%f] sec.\n",runtime);
	}
}

void dijkstra(graph* G, long initial_node, char debug)
{
	long nStart, nEnd;
	long nNodes, nOffset;
	int i;

	MPI_Bcast( G->node[0], G->N*G->N, MPI_CHAR, 0, MPI_COMM_WORLD);


	// We handle the uneven distribution of nodes by assigning the leftovers to the rank 0 for computation 
	nNodes = G->N/size; //Number of nodes for each process
	nOffset = G->N % nNodes; //The first gets more
	if(rank == 0){
		nStart = 0;
		nEnd = nNodes + nOffset;
	} else {
		nStart = (rank * nNodes) + nOffset;
		nEnd = nStart + nNodes;
	}


	if( debug == 1){
		printf("[%d] Processing node [%d-%d]\n",rank, nStart, nEnd);
	}


	// This is used to call the parallel dijkstra's function where each processor does its assigned part of the computation 
	par_dijkstra(G, 0, nStart, nEnd, debug);

	if( debug == 1)
	{
		printf("[%d] Partial D result\n",rank);
		printf("[%d] [", rank);
		for(i = nStart; i < nEnd; i++){
			printf("%03d,",G->D[i]);
		}
		printf("]");
	}

	if(rank == 0){
		printf(" Complete D result [");
		if(G->N > 20)
			nEnd = 20;
		else
			nEnd = G->N;
		for(i = 0; i < nEnd; i++){
			printf("%d,",G->D[i]);
		}
		if(G->N > 20)
			printf(" ... ]");
		else
			printf("]");
	}
}

void par_dijkstra(graph* G, long initial_node, long nStart, long nEnd, char debug)
{
	long i,j,k;
	int aN; //actualNode
	int nextAN[2];

	nextAN[1] = initial_node;
	nextAN[0] = 0;

	if(debug){
		printf("Calculating nodes [%d - %d]\n", nStart, nEnd);
		printf("Running dijkstra on graph\n");
		printGraph(G);
	}

	
   	#pragma omp parallel for schedule(runtime) private(i)
	for(i = 0; i < G->N; i++)
	{
		aN = nextAN[1];
		G->D[aN] = nextAN[0];
		G->visited[aN] = VISITED;

		if(debug){
			printf("[%d] It[%d] aN [%d]\n",rank, i, aN); printStatus(G); printf("\n");
		}

		//Find all nodes connected to aN
		#pragma omp parallel for schedule(runtime) private(j)
		for(j=nStart;j<nEnd;j++){
			if( (G->node[aN][j] != NO_CONN) ){
				if( (G->D[aN] + G->node[aN][j]) < G->D[j] ){
					G->D[j] = (G->D[aN] + G->node[aN][j]);
				}
			}
		}

		//the nextNode here is unlike the OPENMP implementation due to the nStart and nEnd values associated with every processor 

		nextAN[1] = par_NextNode(G, nStart, nEnd); //Which node
		if(nextAN[1] == -1){
			nextAN[0] = INF;
			nextAN[1] = 0; //-1 cant bet set as index for MPI_MINLOC operation, so set INF as value so it wont be chosen
		} else {
			nextAN[0] = G->D[nextAN[1]];				  //Distance of this node
		}


		//MPI_ALLreduce with MPI_MINLOC used to find the global min in nextAN
		MPI_Allreduce(nextAN,nextAN,1,MPI_2INT,MPI_MINLOC,MPI_COMM_WORLD);

	}
}

void printUsage(int argc, char** argv){
	printf("Usage: %s NUMER_OF_POINTS [SRAND_INIT_VALUE]\n",argv[0]);
}

//Used to parse the input commandline arguments to obtain the Init for the rand() function and the size N of the graph
//We have used various error handling mechanisms which we had picked up from a previous implementation of the serial dijkstra's algorithm 
void InputParse(int argc, char** argv)
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

		if ((errno == ERANGE && (Init == LONG_MAX ||Init == LONG_MIN)) || (errno != 0 && Init == 0)) {
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
