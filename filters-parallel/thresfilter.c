#include "thresfilter.h"
#include <mpi.h>

void thresfilter(const int xsize, const int ysize, pixel* src){
#define uint unsigned int 

  uint rank, size;
  uint sum, i, k, psum, nump = 0;
  uint start, stop;
  uint threads = 16;
  nump = xsize * ysize; //Area
  uint chunk = nump/16;

  uint localsum = 0;
  uint globalsum = 0;

  // Thread this shit
  MPI_INIT(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Give the threads coordinates for each segment to work on

  // Calculate start position in array
  start = rank*chunk;
  // Calculate stop position in array
  stop = rank*chunk + chunk;

  // If the picture is not dividable with 16
  if(rank == 15 && (stop != nump)){
    stop = nump;
  }

  localsum = calcThreshold(start, stop, src);
  
  MPI_Reduce(localsum,globalsum,threads,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);

  if(rank == 0){
    printf("globalsum = %d \n",globalsum);
  }

  // Gather all threads
  //Barrier

  // sum = sum / nump;
  //sum /= nump; // Average pixel color
  //scatter
  //apply filter
  //gather threads
  //barrier

  /*
  // Thread this shit
  for(i = 0; i < nump; i++) {
    psum = (uint)src[i].r + (uint)src[i].g + (uint)src[i].b;
    
    // If darker than average pixel color ---> Make pixel black
    if(sum > psum) {
      src[i].r = src[i].g = src[i].b = 0;
    }
    // Brighter than average pixel color --> Make white
    else {
      src[i].r = src[i].g = src[i].b = 255;
    }
  }
  */

  MPI_Finalize();
}

// Calculates the summation of all pixel values from start to stop
int calcThreshold(const int start, const int stop, const pixel* src){

  uint sum = 0;

  // Thread this shit
  for(uint i = start, sum = 0; i < stop; i++) {
    sum += (uint)src[i].r + (uint)src[i].g + (uint)src[i].b;
  }

  return sum;
}
