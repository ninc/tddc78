#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ppmio.h"
#include "thresfilter.h"

#include "mpi.h"

typedef struct _pixel {
    unsigned char r,g,b;
} pixel;

int thresfilteravgpix(const int xsize, const int ysize, pixel* src){
#define uint unsigned int 

  uint sum, i, psum, nump;

  nump = xsize * ysize; //Area

  for(i = 0, sum = 0; i < nump; i++) {
    sum += (uint)src[i].r + (uint)src[i].g + (uint)src[i].b;
  }

  return sum /= nump; // Average pixel color
}

int main (int argc, char ** argv) {
    int xsize, ysize, colmax;
    pixel src[MAX_PIXELS];
    struct timespec stime, etime;
    
    int rank, size;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* Take care of the arguments */

    
    if (argc != 3) {
      fprintf(stderr, "Usage: %s infile outfile\n", argv[0]);
      exit(1);
    }
    
    
    if(myid==0) {
      /* read file */
      if(read_ppm (argv[1], &xsize, &ysize, &colmax, (char *) src) != 0)
	exit(1);

      if (colmax > 255) {
	fprintf(stderr, "Too large maximum color-component value\n");
	exit(1);
      }
      
      
    }
    printf("Has read the image, calling filter\n");

    clock_gettime(CLOCK_REALTIME, &stime);

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

    printf("Filtering took: %g secs\n", (etime.tv_sec  - stime.tv_sec) +
	   1e-9*(etime.tv_nsec  - stime.tv_nsec)) ;

    /* write result */
    printf("Writing output file\n");
    
    if(write_ppm (argv[2], xsize, ysize, (char *)src) != 0)
      exit(1);
       

    return(0);
}
