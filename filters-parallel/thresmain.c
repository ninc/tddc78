#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ppmio.h"
#include "thresfilter.h"

#include "mpi.h"

#define uint unsigned int 

uint sum_avg_pix(const int size, pixel* src){

  uint sum, i;

  for(i = 0, sum = 0; i < size; i++) {
    sum += (uint)src[i].r + (uint)src[i].g + (uint)src[i].b;
  }

  return sum;
}

int main (int argc, char ** argv) {

    int xsize, ysize, colmax;
    int chunk;
    pixel src[MAX_PIXELS];
    pixel* rbuf;
    struct timespec stime, etime;
    int rank, size;
    uint *global_sum;
    uint *local_sum;
    uint avg = 0;
    int root = 0;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* Take care of the arguments */

    
    if (argc != 3) {
      fprintf(stderr, "Usage: %s infile outfile\n", argv[0]);
      MPI_Finalize();
      exit(1);
    }
    
    
    if(rank==root) {
      /* read file */
      if(read_ppm (argv[1], &xsize, &ysize, &colmax, (char *) src) != 0)
	{
	  MPI_Finalize();
	  exit(2);
	}
      if (colmax > 255) {
	fprintf(stderr, "Too large maximum color-component value\n");
	MPI_Finalize();
	exit(3);
      }
      
      
      printf("Has read the image, calling filter\n");

    }

    clock_gettime(CLOCK_REALTIME, &stime);


    // Calc and broadcast chunks
    if(rank==root)
      {
      //Calc chunks
      chunk = (xsize*ysize)/size;
      global_sum = (uint *)malloc(sizeof(uint));
      *global_sum = 0;
      }

    MPI_Bcast(&chunk, 1, MPI_INT, root, MPI_COMM_WORLD);

    if(rank==root)
      printf("Has broadcasted chunks. Calling Scatter\n");


    //Create local buffer
    rbuf = (pixel *)malloc(chunk*sizeof(pixel)); 

    MPI_Scatter(src, chunk, MPI_CHAR, rbuf, chunk, MPI_CHAR, root, MPI_COMM_WORLD);

    if(rank==root)
      printf("Scatter complete\n");

    //Create local buffer
    local_sum = (uint *)malloc(sizeof(uint));
    *local_sum = sum_avg_pix(chunk, rbuf);

    printf("Local_sum: %d\n", *local_sum) ;

    MPI_Reduce(local_sum, global_sum, 1, MPI_UNSIGNED, MPI_SUM, root, MPI_COMM_WORLD);
    
    if(rank==root)
      {
	printf("Global_sum: %d\n", *global_sum);
	avg = *global_sum / (xsize*ysize);
	printf("Avg: %d\n", avg);
      }
    
    //if(myid==0) {
    //Do broadcast
    //The srbuf consists of xsize, ysize and the src vector...need an own created buffer for this shit...
    //Dont know what to do with MPI Datatype really, create an own typedef struct to describe the srbuf or...
    //MPI Bcast( void *srbuf, int count, MPI Datatype datatype, int rootrank, MPI Comm comm );
    // }
    //
    //Dependening on which rank, it executes its avg pix of it
    //if(myid==0) {
    //thresfilteravgpix(startpos for rank 0, end pos for rank 0, the whole src);
    //}
    //if(myid==1) {
    //thresfilteravgpix(startpos for rank 1, end pos for rank 1, the whole src);
    //}
    //...and so on... Not really flexible when trying to run 32 threads, need looots of if statements...
    //MPI_Barrier(MPI_COMM_WORLD);
    //MPI Reduce( void *sbuf, void *rbuf, int count, MPI Datatype datatype, MPI Op op, int rootrank, MPI Comm comm );
    //if(myid==0) {
    //calculate total avg for all segments with data from rbuf...
    //MPI_Barrier(MPI_COMM_WORLD);
    //After this shit > do a broadcast again from rank 0, but with the avg pix color value to follow for calc. of threshold for each thread to do on its own segment...analog thinking as before...
    //Dont know if all this needs some barriers for safe data, might be a good idea...
    
    clock_gettime(CLOCK_REALTIME, &etime);

    if(rank==0)
      {
	printf("Filtering took: %g secs\n", (etime.tv_sec  - stime.tv_sec) +
	       1e-9*(etime.tv_nsec  - stime.tv_nsec)) ;

	/* write result */
	printf("Writing output file\n");
    
	if(write_ppm (argv[2], xsize, ysize, (char *)src) != 0)
	  {
	    MPI_Finalize();
	    exit(4);
	  }
      }

    MPI_Finalize();
    return(0);
}
