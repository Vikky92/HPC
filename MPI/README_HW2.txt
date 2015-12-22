// THIS ASSIGNMENT IS DONE BY MUROOR VIKAS RAO 62805822
    AND RONIT SHYAM KADAM 81365198

The first part of the Homework was to implement to the two load balancing strategies implemented by the two interns

Joe Block has used the computation technique of dividing the computations into blocks per process and calculating the values from the mandelbrot function
This is then given to the render function to get the image.

The Serial Implementation takes around .57 s for a 1000 * 1000 computation
However, the parallel implementation took .0019 s for the same.  

For the Mandelbrot set, the calculation is much slower in the interior of the set, than in the exterior.

Dividing the 2D image into blocks simply gives portions of varying computational difficulty to different processes
This isn't the best possible technique for load balancing.

However, what Susie Cyclic performs is more efficient in terms of load balancing as adjacent blocks have a relatively closer computational difficulty as compared to the blocks or chunks of image.
Hence, in Susie Cyclic's technique we move through the rows of the image in a cyclic order, making every processes compute the adjacent blocks leading to better load balancing 

Thanks to her effort for better load balancing, I would recommend Susie Cyclic to the hiring department. She surely did a more efficient balancing of the computations. 

However, when the number of jobs is much greater than the number of processes we resort to using a Master Slave Technique. 
Here, a master has been designed on the lines of organizing workers and that the master does no work apart from telling the other tasks, the slaves, what to do.

The code works on the lines of sending the slave messages on which row number to work on.
When a slave finishes a row, it will send the result back and await another row if any exists any.
TAGS have been used for this purpose to indicate the status of the process as terminated or not. 

The master then renders the calculated Mandelbrot values for the rendering of the image. 
The slaves sit in a single infinite loop. They only exit when they receive that TERMINATE tag from slave that is if an row for computation doesn't exist.
For very large image sizes, the Master/Slave technique proves to be more efficient than the previous implementations
Scaling would be more efficient with this master/slave approach as the dynamic allocation of jobs takes place.


—————————————————————————————

DotProduct

The elements of the array have been randomly initialized and the elements is given as an argument. This is done for each processor. 

These arrays are subdivided into parts equal to the number of processors and each processor computes its part of the dot product 

Scatter has been used for this purpose. The gather collective is used for accumulation of these dot products and final result is then computed 

Scatter and gather are the MPI collectives which use the tree algorithm as discussed in class. 
However, the reduce collective could also be used for the same. 

RESULTS 

NUM_PROCS        1       2      4       8       16      32    64     128
TIME TAKEN (s)   0.0087  .010   .0135   .0259   .0252   .018   .49    0.58

