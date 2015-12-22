
#include <iostream>
#include <cstdlib>

#include <mpi.h>
#include "render.hh"

// tags for sends/receives
#define DATA_TAG 1
#define TERMINATE_TAG 0
#define RESULT_TAG 2

using namespace std;

void master(int height,int width);

void slave(int width, int height) ;


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
  /* Lucky you, you get to write MPI code */
  
  int numtasks,rank,sendcount,recvcount,source ; 
  
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
 
  MPI_Status status; 
  double minX = -2.1;
  double maxX = 0.7;
  double minY = -1.25;
  double maxY = 1.25;
  
          double start = MPI_Wtime(); 
     
  int  height, width;
  if (argc == 3) {
    height = atoi (argv[1]);
    width = atoi (argv[2]);
    assert (height > 0 && width > 0);
  } else {
    fprintf (stderr, "usage: %s <height> <width>\n", argv[0]);
    fprintf (stderr, "where <height> and <width> are the dimensions of the image.\n");
    return -1;
  }
   

  double it = (maxY - minY)/height;
  double jt = (maxX - minX)/width;
  double x,y;

  	if(rank==0)
  	{
  		master(height,width);
  	}

  	else {
  		slave(width,height);
  	}

 	 MPI_Finalize();
 }

 void master(int height,int width){
        double start = MPI_Wtime();
 	int num_processors ; 
 	MPI_Status status; 
 	int row = 0; 

  double y=0,x=0;
  gil::rgb8_image_t img(height, width);
  auto img_view = gil::view(img);
 
   double minX = -2.1;
  double maxX = 0.7;
  double minY = -1.25;
  double maxY = 1.25;

   double it = (maxY - minY)/height;
  double jt = (maxX - minX)/width;
    
    double* recv = (double*)malloc((width)*sizeof(double));

  //  double* mandel = (double*)malloc((width+1)*height*sizeof(double));
     double mandel[height+1][width+1];
 	MPI_Comm_size(MPI_COMM_WORLD,&num_processors);

 	for(int i =1 ;i<num_processors;i++){
 		MPI_Send(&row,1,MPI_INT,i,DATA_TAG,MPI_COMM_WORLD);
 		row++;
 	}

 	int doneRows = 0 ;

 	while(doneRows<height){
 MPI_Recv(recv,width,MPI_DOUBLE,MPI_ANY_SOURCE,RESULT_TAG,MPI_COMM_WORLD,&status);
 		
 		int doneSlave = status.MPI_SOURCE;
 		int receivedRow = recv[0];
 		for(int col = 0; col<width;col++)
 		    mandel[receivedRow][col]=recv[col+1] ;
               
		doneRows++;
 		if(row<height){
 		    MPI_Send(&row,1,MPI_INT,doneSlave,DATA_TAG,MPI_COMM_WORLD);
 				row++;

 			}


 	}
 
 	for(int i =1 ;i<num_processors;i++)
 		MPI_Send(0,0,MPI_INT,i,TERMINATE_TAG,MPI_COMM_WORLD);
 
	

        for (int i = 0; i < height; ++i) {
    		for (int j = 0; j < width; ++j) 
	 		img_view(i, j) = render(mandel[i][j]);
      
    } 
  
  	gil::png_write_view("mandelbrot1.png", const_view(img));
      double end = MPI_Wtime();
      cout<<end - start <<endl; 
 }
  

 void slave(int width, int height) {

   double minX = -2.1;
  double maxX = 0.7;
  double minY = -1.25;
  double maxY = 1.25;


   double it = (maxY - minY)/height;
  double jt = (maxX - minX)/width;

  double y=0,x=0;
 	int row =0;
    double* send = (double*)malloc((width)*sizeof(double));
 	int slave_rank;
 	MPI_Status status ; 
 	MPI_Comm_rank(MPI_COMM_WORLD,&slave_rank);
 	MPI_Recv(&row,1,MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);

 	while(status.MPI_TAG == DATA_TAG) {

 			if(status.MPI_TAG ==TERMINATE_TAG)
 			 	exit(0);		
 	

 	send[0]=row; 

 	y = minY+row*it;
      for (int i = row; i < row+1; ++i) {
      x = minX;
   
      for (int j = 0; j < width; ++j) {
           send[j+1]=(mandelbrot(x,y)/512.0);
                 x += jt;
         } 
      y += it;
 }

   MPI_Send(send,width,MPI_DOUBLE,0,RESULT_TAG,MPI_COMM_WORLD);
   MPI_Recv(&row,1,MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
 }
}
