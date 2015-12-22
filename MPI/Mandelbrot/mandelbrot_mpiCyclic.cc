#include <iostream>
#include <cstdlib>

#include <mpi.h>
#include "render.hh"

using namespace std;

int
mandelbrot(double x, double y) {
  int maxit = 511;
  double cx = x;
  double cy = y;
  double newx, newy;

  int it = 0;
  for (it = 0; it < maxit && (x*x + y*y) < 4; ++it) {
    newx = x*x - y*y + cx;
    newy = 2*x*y + cy;
    x = newx;
    y = newy;
  }
  return it;
}



int
main (int argc, char* argv[])
{
  
  
  int numtasks,rank,sendcount,recvcount,source ; 
  int  height, width;
  if (argc == 3) {
    height = atoi (argv[1]);
    width = atoi (argv[2]);
    assert (height > 0 && width > 0);
  } 
    else {
    fprintf (stderr, "usage: %s <height> <width>\n", argv[0]);
    fprintf (stderr, "where <height> and <width> are the dimensions of the image.\n");
    return -1;
  }
   
    
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
  int chunksize = height/numtasks;
  MPI_Status status; 
  
  double minX = -2.1;
  double maxX = 0.7;
  double minY = -1.25;
  double maxY = 1.25;
  double* sendbuff = (double*)malloc((width*chunksize)*sizeof(double));
          double start = MPI_Wtime(); 
  double* recvbuff;
   
  double it = (maxY - minY)/height;
  double jt = (maxX - minX)/width;
  double x,y; 

  gil::rgb8_image_t img(height, width);
  auto img_view = gil::view(img);

	// computation done by Cyclic Intern
         y= minY + rank*it;
   for (int i=rank ; i <height ; i+=numtasks) 
    {
         x = minX;
      for (int j = 0; j < width; ++j) 
      {
          
          img_view(i,j)=render(mandelbrot(x,y)/512.0);
                 x += jt;
      } 
      y += it*numtasks;
    }
  //Sync all the processes 
   MPI_Barrier(MPI_COMM_WORLD);          
 

   gil::png_write_view("mandelbrot1.png", const_view(img));
MPI_Finalize();

}

/* eof */
