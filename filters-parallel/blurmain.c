#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ppmio.h"
#include "blurfilter.h"
#include "gaussw.h"
#include "mpi.h"


#define MAX_RAD 1000

int main (int argc, char ** argv) {
  int radius;
  int xsize, ysize, colmax;
  pixel src[MAX_PIXELS];
  pixel dst[MAX_PIXELS];
  pixel* rbuf = NULL;
  pixel* local_dst = NULL;
  struct timespec stime, etime;

  double w[MAX_RAD];

  //pixel dst[MAX_PIXELS];
  int rank, size;
  int root = 0;
  int y;
  

    MPI_Init(NULL, NULL);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* create a type for struct car */
    const int nitems=3;
    int blocklengths[3] = {1,1,1};

    MPI_Datatype types[3] = {MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR};
    // MPI_Datatype types[3] = {MPI_CHAR, MPI_CHAR, MPI_CHAR};
    MPI_Datatype mpi_img;
    MPI_Aint offsets[3];
    
    offsets[0] = offsetof(pixel, r);
    offsets[1] = offsetof(pixel, g);
    offsets[2] = offsetof(pixel, b);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_img);
    MPI_Type_commit(&mpi_img);

    //Global
    int displc[size];
    int sendcount[size];
    int recimage[size];
    int recelement[size];
    // Local variables
    int displ;
    int displ_down;
    int sendcnt;
    int recimg;
    int recelm;

    /* Take care of the arguments */

    if (argc != 4) {
      if(rank==root)
	fprintf(stderr, "Usage: %s radius infile outfile\n", argv[0]);
      MPI_Finalize();
      exit(1);
    }

    if(rank==root)
      {
	radius = atoi(argv[1]);
	if((radius > MAX_RAD) || (radius < 1)) {
	  fprintf(stderr, "Radius (%d) must be greater than zero and less then %d\n", radius, MAX_RAD);
	  MPI_Finalize();
	  exit(1);
	}

	/* read file */
	if(read_ppm (argv[2], &xsize, &ysize, &colmax, (char *) src) != 0){
	  MPI_Finalize();
	  exit(1);
	}

	if (colmax > 255) {
	  fprintf(stderr, "Too large maximum color-component value\n");
	  MPI_Finalize();
	  exit(1);
	}

	printf("Has read the image, generating coefficients\n");

	/* filter */
	get_gauss_weights(radius, w);

	printf("Calling filter\n");

	y = (ysize/size);
	printf("value of y and x: %d, %d \n", y, xsize);
      }

    //Creating buffer
    
    MPI_Bcast(&radius, 1, MPI_INT, root, MPI_COMM_WORLD);
    
    MPI_Bcast(&w, MAX_RAD, MPI_DOUBLE, root, MPI_COMM_WORLD);
    
    MPI_Bcast(&xsize, 1, MPI_INT, root, MPI_COMM_WORLD);
    MPI_Bcast(&y, 1, MPI_INT, root, MPI_COMM_WORLD);
    MPI_Bcast(&ysize, 1, MPI_INT, root, MPI_COMM_WORLD);

    
    // Calculate distribution of threads
    //Top layer
    if(rank == root)
      {
	displ = 0;

	sendcnt = (y+radius)*xsize; 
	
	if(sendcnt > xsize*ysize)
	  sendcnt = xsize*ysize;

	displ_down = ysize - displ + y;
	recimg = rank*y*xsize;
	recelm = y * xsize;
      }
    //Bottom layer
    else if(rank==size-1)
      {
	
	displ = rank*(y-radius)*xsize;

	if(displ < 0)
	  displ = 0;
	displ_down = ysize - displ + y;
	sendcnt = ((ysize-y*size + y)+radius)*xsize;

	if(sendcnt > xsize*ysize)
	  sendcnt = xsize*ysize;


	recimg = rank*y*xsize;
	recelm = (ysize-y*size + y)* xsize;
      }
    //Middle layers
    else
      {
	displ = rank*(y-radius)*xsize;

	if(displ < 0)
	  displ = 0;
	displ_down = ysize - displ + y;
	sendcnt = (y+2*radius)*xsize;

	if(sendcnt > xsize*ysize)
	  sendcnt = xsize*ysize;

	recimg = rank*y*xsize;
	recelm = y*xsize;
      }

    //printf("Local displacement for rank %u: %u\n", rank, displ);
    //printf("Send count for rank %u: %u\n", rank, sendcnt);


    //Create read buffer
    rbuf = (pixel *)malloc(sendcnt*sizeof(pixel)); 
    local_dst = (pixel *)malloc(recelm*sizeof(pixel));


    //Recieve global distribution of threads
    MPI_Gather(&displ, 1, MPI_INT, displc, 1, MPI_INT, root, MPI_COMM_WORLD);
    MPI_Gather(&sendcnt, 1, MPI_INT, sendcount, 1, MPI_INT, root, MPI_COMM_WORLD);
    MPI_Gather(&recimg, 1, MPI_INT, recimage, 1, MPI_INT, root, MPI_COMM_WORLD);
    MPI_Gather(&recelm, 1, MPI_INT, recelement, 1, MPI_INT, root, MPI_COMM_WORLD);

    // Where the action happens!
    clock_gettime(CLOCK_REALTIME, &stime);

    // Scatter the fucking whore bitch ass cunt
    MPI_Scatterv(src, sendcount, displc, mpi_img, rbuf, sendcnt, mpi_img, root, MPI_COMM_WORLD);
    //MPI_Scatter(dst, y*xsize, mpi_img, local_dst, y*xsize, mpi_img, root, MPI_COMM_WORLD);
  


    //For testing purposes only
    /*
    int j;
    for(j = 0; j<recelm; j++)
      {
	local_dst[j].r = rank*(255/size);
	local_dst[j].g = 0;
	local_dst[j].b = 0;
      }
    
    */

    //Handle larger chunk at the bottom
    if(rank==size-1)
      blur(xsize, (y + ysize - y*size), src, local_dst, radius, w, displ, displ_down, recelm);
    else
      blur(xsize, y, src, local_dst, radius, w, displ, displ_down, recelm);


    //blur_y(xsize, y, src, local_dst, radius, w, displ, displ_down);

     

    if(rank==size-1)
      {
	uint k = recimg + recelm;
	uint o = xsize*ysize;

	printf("last_pos %d, imagesize %d\n",k,o);

      }

    
    // Scatter the fucking whore bitch ass cunt
    //printf("Rank : %d, recimg: %d \n", rank, recimg);
   
    MPI_Gatherv(local_dst, recelm, mpi_img, src, recelement, recimage, mpi_img, root, MPI_COMM_WORLD);

    clock_gettime(CLOCK_REALTIME, &etime);

    if(rank==root)
      {
	printf("Filtering took: %g secs\n", (etime.tv_sec  - stime.tv_sec) +
	       1e-9*(etime.tv_nsec  - stime.tv_nsec)) ;

	/* write result */
	printf("Writing output file\n");
    
	if(write_ppm (argv[3], xsize, ysize, (char *)src) != 0){
	  MPI_Finalize();
	  exit(1);
	}
      }

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();
    return(0);
}
