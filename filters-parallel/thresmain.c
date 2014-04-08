#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ppmio.h"
#include "thresfilter.h"
#include <mpi.h>

#define uint unsigned int 

int main (int argc, char ** argv) {
    int xsize, ysize, colmax;
    pixel src[MAX_PIXELS];
    struct timespec stime, etime;



    int rank, size, threshold;
    uint sum, i, k, psum, nump = 0;
    uint start, stop;
    uint localsum = 0;
    uint globalsum = 0;
    uint chunk = 0;

    // Thread this shit
    MPI_INIT();
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0)
      {
	/* Take care of the arguments */

	if (argc != 3) {
	  fprintf(stderr, "Usage: %s infile outfile\n", argv[0]);
	  exit(1);
	}

	/* read file */
	if(read_ppm (argv[1], &xsize, &ysize, &colmax, (char *) src) != 0)
	  exit(1);

	if (colmax > 255) {
	  fprintf(stderr, "Too large maximum color-component value\n");
	  exit(1);
	}
      
	nump = xsize * ysize; //Area
	chunk = nump / size;
	printf("Has read the image, calling filter\n");
      }

    
    // Wait for file to be read
    MPI_Barrier(MPI_COMM_WORLD);

    clock_gettime(CLOCK_REALTIME, &stime);
    
    //Scatter
    

    // Give the threads coordinates for each segment to work on

    // Calculate start position in array
    start = rank*chunk;
    // Calculate stop position in array
    stop = rank*chunk + chunk;

    // If the picture is not dividable with number of processes
    if(rank == size-1 && (stop != nump)){
      stop = nump;
    }

    localsum = calcThreshold(start, stop, src);
  
    MPI_Reduce(&localsum,&globalsum,size,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);

    // Print the threshold value
    if(rank == 0){
      threshold = globalsum / nump;
      printf("globalsum / nump = threshold: %d / %d = % d \n",globalsum, nump, threshold);
    }
       
    MPI_Barrier(MPI_COMM_WORLD);

    // Apply the filter
    thresfilter(start, stop, threshold, src);

    MPI_Barrier(MPI_COMM_WORLD);


    // Write Image to disk
    if(rank == 0)
      {
	clock_gettime(CLOCK_REALTIME, &etime);

	printf("Filtering took: %g secs\n", (etime.tv_sec  - stime.tv_sec) +
	       1e-9*(etime.tv_nsec  - stime.tv_nsec)) ;

	/* write result */
	printf("Writing output file\n");
    
	if(write_ppm (argv[2], xsize, ysize, (char *)src) != 0)
	  exit(1);
      }


    MPI_Finalize();

    return(0);
}
