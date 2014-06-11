#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ppmio.h"
//#include "blurfilter.h"
#include "gaussw.h"
#define NUM_THREADS 16
#define MAX_RAD 1000

// x_matfe
// HPL2245wg

////////////////////////////
pthread_barrier_t barr;

typedef struct _pixel {
    unsigned char r,g,b;
} pixel;


typedef struct _blur_data
{
	int id;
	unsigned int x_start;
	unsigned int x_end;
	unsigned int y_start;
	unsigned int y_end;
	unsigned int y_max;
	unsigned int x_max;
	unsigned int img_start;
	unsigned int img_end;
	unsigned int radius;
	double * gauss;
	pixel* src;
	pixel* dst;
} blur_data;

//////////////////////////////////////////////////////////

pixel* pix(pixel* image, const int xx, const int yy, const int xsize)
{
  register int off = xsize*yy + xx;

#ifdef DBG
  if(off >= MAX_PIXELS) {
    fprintf(stderr, "\n Terribly wrong: %d %d %d\n",xx,yy,xsize);
  }
#endif
  return (image + off);
}



void* blur(void *d){
  blur_data *data = (blur_data*) d;
  pixel* src = data->src;
  double* w = data->gauss;
  unsigned int xsize = data->x_max;
  unsigned int ysize = data->y_max;
  unsigned int radius = data->radius;
  int x, y, x2, y2, wi;
  double r, g, b, n, wc;
  pixel *dst = data->dst;

  printf("y_start %d, y_end %d, radius %d, img_start %d, img_end %d of thread %d\n", data->y_start, data->y_end, data->radius, data->img_start, data->img_end, data->id);

	for (y = data->img_start; y<=data->img_end; y++) {
		for (x = data->x_start; x<data->x_end; x++) {
			r = w[0] * pix(src, x, y, xsize)->r;
			g = w[0] * pix(src, x, y, xsize)->g;
			b = w[0] * pix(src, x, y, xsize)->b;
			n = w[0];
			for (wi = 1; wi <= radius; wi++) {
				wc = w[wi];
				x2 = x - wi;
				// If we are outside of the img
				if (x2 >= 0) {
					r += wc * pix(src, x2, y, xsize)->r;
					g += wc * pix(src, x2, y, xsize)->g;
					b += wc * pix(src, x2, y, xsize)->b;
					n += wc;
				}
				x2 = x + wi;
				// If we are outside of the img
				if (x2 <= data->x_max) {
					r += wc * pix(src, x2, y, xsize)->r;
					g += wc * pix(src, x2, y, xsize)->g;
					b += wc * pix(src, x2, y, xsize)->b;
					n += wc;
				}
			}
			pix(dst, x, y, xsize)->r = r / n;
			pix(dst, x, y, xsize)->g = g / n;
			pix(dst, x, y, xsize)->b = b / n;
		}
	}

	// Synchronization point
	int rc = pthread_barrier_wait(&barr);
	if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
	  {
	  printf("Could not wait on barrier\n");
	   exit(-1);
	  }

	for (y = data->img_start; y<=data->img_end; y++) {
		for (x = data->x_start; x<data->x_end; x++) {
			r = w[0] * pix(dst, x, y, xsize)->r;
			g = w[0] * pix(dst, x, y, xsize)->g;
			b = w[0] * pix(dst, x, y, xsize)->b;
			n = w[0];
			for (wi = 1; wi <= radius; wi++) {
				wc = w[wi];
				y2 = y - wi;
				// If we are outside of the img
				if (y2 >= 0 && y2 >= data->y_start) {
					r += wc * pix(dst, x, y2, xsize)->r;
					g += wc * pix(dst, x, y2, xsize)->g;
					b += wc * pix(dst, x, y2, xsize)->b;
					n += wc;
				}
				y2 = y + wi;
				// If we are outside of the img
				if (y2 <= data->y_end && y2 <= data->y_max) {
					r += wc * pix(dst, x, y2, xsize)->r;
					g += wc * pix(dst, x, y2, xsize)->g;
					b += wc * pix(dst, x, y2, xsize)->b;
					n += wc;
				}
			}
			pix(data->src, x, y, xsize)->r = r / n;
			pix(data->src, x, y, xsize)->g = g / n;
			pix(data->src, x, y, xsize)->b = b / n;
		}
	}

	return 0;
}









//------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void calc_thread_data(blur_data data[], int xsize, int ysize, int radius, double* gauss, pixel* src, pixel* dst)
{
  unsigned int chunk_size = ysize / NUM_THREADS;
  unsigned int start = 0;
  unsigned int end = chunk_size;
  int y_start;
  int y_end;
  int i = 0;

  //Assign relevant data to each thread
  for (i = 0; i < NUM_THREADS; i++)
    {

      // Correct the concatination fault
      if (i == NUM_THREADS - 1)
	{
	  end = ysize-1;
	}
      else
	{
	  end = start + chunk_size;
	}

      data[i].id = i;
      data[i].x_start = 0;
      data[i].x_end = xsize;
      data[i].img_start = start;
      data[i].img_end = end;
      data[i].y_max = ysize;
      data[i].x_max = xsize;
      data[i].radius = radius;
      data[i].gauss = gauss;
      data[i].src = src;
      data[i].dst = dst;



      y_start = start - radius;
      y_end = end + radius;

      // Correct out of bounds radius
      if (y_start < 0)
	{
	  y_start = 0;
	}

      // Correct out of bounds radius
      if (y_end > ysize)
	{
	  y_end = ysize;
	}
		

      data[i].y_start = y_start;
      data[i].y_end = y_end;

      // Assign next chunk
      start = end + 1;

    }
}

int main (int argc, char ** argv) {
  int radius;
  int xsize, ysize, colmax;
  pixel src[MAX_PIXELS];
  pixel dst[MAX_PIXELS];
  struct timespec stime, etime;
  blur_data data[NUM_THREADS];
  pthread_t threads[NUM_THREADS];
  long t = 0;
  int ret;

  double w[MAX_RAD];

  // Barrier initialization
  if(pthread_barrier_init(&barr, NULL, NUM_THREADS))
    {
      printf("Could not create a barrier\n");
      exit(1);
    }

  /* Take care of the arguments */

  if (argc != 4) {
    fprintf(stderr, "Usage: %s radius infile outfile\n", argv[0]);
    exit(1);
  }
  radius = atoi(argv[1]);
  if((radius > MAX_RAD) || (radius < 1)) {
    fprintf(stderr, "Radius (%d) must be greater than zero and less then %d\n", radius, MAX_RAD);
    exit(1);
  }

  /* read file */
  if(read_ppm (argv[2], &xsize, &ysize, &colmax, (char *) src) != 0)
    exit(1);

  if (colmax > 255) {
    fprintf(stderr, "Too large maximum color-component value\n");
    exit(1);
  }

  //printf("Has read the image, generating coefficients\n");

  /* filter */
  get_gauss_weights(radius, w);

  //printf("Setting up threads\n");

  calc_thread_data(data, xsize, ysize, radius, w, src, dst);

  printf("Calling filter\n");

  //clock_t t1,t2;
   struct timespec start, stop;
  // clock_gettime(CLOCK_MONOTONIC, &start);
  // long long int t1 = start.tv_sec * 1000000000 + start.tv_nsec;
  //struct timespec start, stop;
  //clock_gettime(CLOCK_MONOTONIC, &stop);
  clock_gettime(CLOCK_MONOTONIC, &start);
  long long int t1 = start.tv_sec * 1000000000 + start.tv_nsec;
  //t1 = clock();
  //clock_gettime(CLOCK_REALTIME, &stime);

  //Apply blur filter for each threads segment
  for (t = 0; t<NUM_THREADS; t++){
    //printf("Initiating thread: %ld\n", t);
    ret = pthread_create(&threads[t], NULL, blur, (void*)&data[t]);
    if (ret){
      printf("Failed to initiate thread: %ld\n!", t);
      exit(-1);
    }
  }
  for (t = 0; t<NUM_THREADS; t++){
    pthread_join(threads[t], NULL);
  }

   clock_gettime(CLOCK_MONOTONIC, &stop);
   long long int t2 = stop.tv_sec * 1000000000 + stop.tv_nsec;
  //clock_gettime(CLOCK_MONOTONIC, &stop);
  //long long int t2 = stop.tv_sec * 1000000000 + stop.tv_nsec;
  //t2 = clock();
  float sim_time = (double)(t2-t1)/1000000000;
  //float diff = (((float)t2 - (float)t1) /  CLOCKS_PER_SEC) * 1000; 

  printf("Filtering took [s] : %f \n", sim_time);

  //blurfilter(xsize, ysize, src, radius, w);

  //clock_gettime(CLOCK_REALTIME, &etime);

  /*
    printf("Filtering took: %g secs\n", (etime.tv_sec  - stime.tv_sec) +
    1e-9*(etime.tv_nsec  - stime.tv_nsec)) ;
  */
  /* write result */
  printf("Writing output file\n");
    
  //Write the blurred img
  if(write_ppm (argv[3], xsize, ysize, (char *)src) != 0)
    exit(1);


  return(0);
}
