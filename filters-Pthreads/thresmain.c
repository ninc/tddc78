#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ppmio.h"
#include "thresfilter.h"
#include <pthread.h>
#DEFINE NUM_THREADS 4

int main (int argc, char ** argv) {
    int xsize, ysize, colmax;
    pixel src[MAX_PIXELS];
    struct timespec stime, etime;

    pthread_t threads[NUM_THREADS];
    long t;
    int ret;
    //Initiate the threads

    for(t=0;t<NUM_THREADS;t++){
      printf("Initiating thread: %ld\n", t);
      ret = pthread_create(&threads[t], NULL, PrintHello, (void)*t);
      if(ret){
	printf("Failed to initiate thread: %ld\n!", t);
	exit(-1);
      }
    }
    for(t=0;t<NUM_THREADS;t++){
      pthread_join(thread[t], NULL);
    }
    
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

    printf("Has read the image, calling filter\n");

    clock_gettime(CLOCK_REALTIME, &stime);

    thresfilter(xsize, ysize, src);

    clock_gettime(CLOCK_REALTIME, &etime);

    printf("Filtering took: %g secs\n", (etime.tv_sec  - stime.tv_sec) +
	   1e-9*(etime.tv_nsec  - stime.tv_nsec)) ;

    /* write result */
    printf("Writing output file\n");
    
    if(write_ppm (argv[2], xsize, ysize, (char *)src) != 0)
      exit(1);


    return(0);
}
