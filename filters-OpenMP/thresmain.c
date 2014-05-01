#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ppmio.h"
#include "thresfilter.h"
#include "omp.h"

int main (int argc, char ** argv) {
  int xsize, ysize, colmax;
  pixel src[MAX_PIXELS];
  struct timespec stime, etime;

  /* Take care of the arguments */
#pragma omp master
  {
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

    printf("Has read the image, calling filter\n");
    
    printf("Hello from master\n");
  }
#pragma omp parallel
  {
    printf("Hello, my name is: %d\n", omp_get_thread_num());
    //clock_gettime(CLOCK_REALTIME, &stime);
  }
  
  
  uint sum, i, psum, nump;
  nump = xsize * ysize; //Area
  sum = 0;
#pragma omp parallel for reduction( +: sum) shared(src, nump) private(i)
  for(i = 0; i < nump; i++) {
    sum += (uint)src[i].r + (uint)src[i].g + (uint)src[i].b;
  }

  // sum = sum / nump;
  sum /= nump; // Average pixel color

  printf("Avg: %d\n", sum);

#pragma omp flush(sum)
#pragma omp parallel for shared(src, sum, nump) private(i, psum)
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
  
    
    //thresfilter(xsize, ysize, src);
  
  //clock_gettime(CLOCK_REALTIME, &etime);

  //  printf("Filtering took: %g secs\n", (etime.tv_sec  - stime.tv_sec) +
  //	 1e-9*(etime.tv_nsec  - stime.tv_nsec)) ;

  /* write result */
  printf("Writing output file\n");
    
  if(write_ppm (argv[2], xsize, ysize, (char *)src) != 0)
    exit(1);


  return(0);
}








