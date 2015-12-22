#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>

//Creates random numbers in an array for computation
float *create_rand_nums(int num_elements)
{
  float *rand_nums = (float *)malloc(sizeof(float) * num_elements);
  assert(rand_nums != NULL);
  int i;
  for (i = 0; i < num_elements; i++)
  {
    rand_nums[i] = (rand() / (float)RAND_MAX);
  //  printf("Array elements are %f\n", rand_nums[i]);
  }
  return rand_nums;
}

//Used for the final addition of dot products terms generated by each processor
float summer(float *array, int num_elements)
{
 float sum = 0.f;
 int i;
 for (i = 0; i < num_elements; i++)
 {
    sum = sum + (array[i]);
 }
  return sum;
}

//Used to compute the dot product of the subarray allocated to each processor
float dot(float *array1, float *array2, int num_elements)
{
  float sum = 0.f;
  int i;
 for (i = 0; i < num_elements; i++)
 {
    sum = sum + (array1[i]*array2[i]);
 }
  return sum;
}
int main(int argc, char** argv)
{
  if (argc != 2)
  {
    fprintf(stderr, "Usage: avg num_elements_per_proc\n");
    exit(1);
  }

int num_elements_per_proc = atoi(argv[1]);
srand(time(NULL));
double start, end, time = 0.0;

MPI_Init(NULL, NULL);
start = MPI_Wtime();
int world_rank;
MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
int world_size;
MPI_Comm_size(MPI_COMM_WORLD, &world_size);
float *rand_nums_A = NULL;
float *rand_nums_B = NULL;

//Creates random arrays A and B
if (world_rank == 0)
{
  rand_nums_A = create_rand_nums(num_elements_per_proc * world_size);
  rand_nums_B = create_rand_nums(num_elements_per_proc * world_size);
}

//Memory allocated for each subarray
float *sub_rand_nums_A = (float *)malloc(sizeof(float) * num_elements_per_proc);
float *sub_rand_nums_B = (float *)malloc(sizeof(float) * num_elements_per_proc);

assert(sub_rand_nums_A != NULL);
assert(sub_rand_nums_B != NULL);

//Scatter each part of the array to the respective processor
MPI_Scatter(rand_nums_A, num_elements_per_proc, MPI_FLOAT, sub_rand_nums_A, num_elements_per_proc, MPI_FLOAT, 0, MPI_COMM_WORLD);
MPI_Scatter(rand_nums_B, num_elements_per_proc, MPI_FLOAT, sub_rand_nums_B, num_elements_per_proc, MPI_FLOAT, 0, MPI_COMM_WORLD);

//Sends both the sub arrays to the dot product function and returns the partial dot product
float partials = dot(sub_rand_nums_A,sub_rand_nums_B, num_elements_per_proc);

float *products = NULL;
float *finals = NULL;
if (world_rank == 0)
 {
//Allocates memory to store the partial products which are equal to the no. of processors   
   products = (float *)malloc(sizeof(float) * world_size);
    assert(products != NULL);
 }

//Gathers all the partial products 
MPI_Gather(&partials, 1, MPI_FLOAT, products, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);

  if (world_rank == 0)
  {
    //Takes the sum of the partial products generated by each processor and sums up to the total partial product of all the elements
    float avg = summer(products, world_size);
    printf("Dot product of all elements is %f\n", avg);
    float original_data_avg = dot(rand_nums_A,rand_nums_B, num_elements_per_proc * world_size);
    printf("Dot product of all elements  is %f\n", original_data_avg);
   }
//Memory deallocation
if (world_rank == 0)
 {
    free(rand_nums_A);
    free(rand_nums_B);
    free(products);
  }
  free(sub_rand_nums_A);
  free(sub_rand_nums_B);
  MPI_Barrier(MPI_COMM_WORLD);
  end = MPI_Wtime();
  time = end - start;
  printf("Time taken by the respective processor %f\n", time);
  MPI_Finalize();
}
