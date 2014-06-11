#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ppmio.h"
#include "thresfilter.h"
#define NUM_THREADS 16

typedef struct _thresh_data
{
 int id;
 unsigned int start;
 unsigned int end;
 unsigned int average;
 pixel* src;
} thresh_data;

void calc_thread_data(thresh_data data[], int xsize, int ysize, pixel* src)
{
 unsigned int img_size = xsize*ysize;
 unsigned int chunk_size = img_size / NUM_THREADS;
 unsigned int start = 0;
 unsigned int end = chunk_size;
 int i = 0;
 for (i = 0; i < NUM_THREADS; i++)
 {
  data[i].id = i;
  data[i].average = 0;
  data[i].start = start;
  data[i].end = end;
  data[i].src = src;
  start = end + 1;
  

  // Correct the concatination fault
  if (i == NUM_THREADS - 1)
  {
   end = img_size;
  }
  else
  {
   end = start + chunk_size;
  }

 }
}

void sum_avg(thresh_data data[])
{
  unsigned int i = 0;
  unsigned int sum = 0;
  for(i = 0; i < NUM_THREADS; i++)
    {
      sum = sum + data[i].average;
    }
  sum /= NUM_THREADS;
  for(i=0; i< NUM_THREADS; i++)
    {
      data[i].average = sum;
      //printf("average: %d of thread: %d \n", data[i].average, data[i].id); 
    }
}

void* apply_filter(void *d)
{
 
  thresh_data *data = (thresh_data*) d;
  unsigned int size = data->end - data->start;
  unsigned int psum = 0;
  unsigned int i = 0;
  for(i = data->start; i < data->end; i++) {
    psum = (uint)data->src[i].r + (uint)data->src[i].g + (uint)data->src[i].b;
    
    // If darker than average pixel color ---> Make pixel black
    if(data->average > psum) {
      data->src[i].r = data->src[i].g = data->src[i].b = 0;
    }
    // Brighter than average pixel color --> Make white
    else {
      data->src[i].r = data->src[i].g = data->src[i].b = 255;
    }
  }
  return 0;
}


void* calc_average(void *d)
{
 thresh_data *data = (thresh_data*) d;
 unsigned int size = data->end - data->start;
 unsigned int sum = 0;
 unsigned int i = 0;
 for (i = data->start; i < data->end; i++) {
  sum += data->src[i].r + data->src[i].g + data->src[i].b;
 }

 data->average = sum / size; // Average pixel color

 //printf("Avg: %d\n", data->average);

 return 0;
}

int main (int argc, char ** argv) {
    int xsize, ysize, colmax;
    pixel src[MAX_PIXELS];
    struct timespec stime, etime;
    thresh_data data[NUM_THREADS];
    pthread_t threads[NUM_THREADS];
    long t;
    int ret;
    //Initiate the threads
    
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

    calc_thread_data(data, xsize, ysize, src);

    //clock_gettime(CLOCK_REALTIME, &stime);

    // clock_t t1,t2;
    struct timespec start, stop;
    //clock_gettime(CLOCK_MONOTONIC, &stop);
    clock_gettime(CLOCK_MONOTONIC, &start);
    long long int t1 = start.tv_sec * 1000000000 + start.tv_nsec;
 

    //Calculate average data for each threads segment
    for(t=0;t<NUM_THREADS;t++){
      //printf("Initiating thread: %ld\n", t);
      ret = pthread_create(&threads[t], NULL, calc_average, (void*) &data[t]);
      if(ret){
	printf("Failed to initiate thread: %ld\n!", t);
	exit(-1);
      }
    }
    for(t=0;t<NUM_THREADS;t++){
      pthread_join(threads[t], NULL);
    }
    //Calculates total average and share to each thread
    sum_avg(data);

    for(t=0;t<NUM_THREADS;t++){
      //printf("Initiating thread: %ld\n", t);
      ret = pthread_create(&threads[t], NULL, apply_filter, (void*) &data[t]);
      if(ret){
	printf("Failed to initiate thread: %ld\n!", t);
	exit(-1);
      }
    }
    for(t=0;t<NUM_THREADS;t++){
      pthread_join(threads[t], NULL);
    }


    clock_gettime(CLOCK_MONOTONIC, &stop);
    long long int t2 = stop.tv_sec * 1000000000 + stop.tv_nsec;

    //printf("Filtering took: %g secs\n", (etime.tv_sec  - stime.tv_sec) +
    //	   1e-9*(etime.tv_nsec  - stime.tv_nsec)) ;

    float sim_time = (double)(t2-t1)/CLOCKS_PER_SEC;

    printf("Filtering took [s] : %f\n", sim_time);

    /* write result */
    printf("Writing output file\n");
    
    if(write_ppm (argv[2], xsize, ysize, (char *)src) != 0)
      exit(1);


    return(0);
}
