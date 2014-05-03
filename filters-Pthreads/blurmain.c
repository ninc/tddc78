#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ppmio.h"
#include "blurfilter.h"
#include "gaussw.h"
#include <pthread.h>
#define NUM_THREADS 4


// x_matfe
// HPL2245wg

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




void calc_thread_data(blur_data data[], int xsize, int ysize, int radius, double* gauss, pixel* src, pixel* dst)
{
	unsigned int chunk_size = ysize / NUM_THREADS;
	unsigned int start = 0;
	unsigned int end = chunk_size;
	unsigned int y_start;
	unsigned int y_end;
	int i = 0;

	//Assign relevant data to each thread
	for (i = 0; i < NUM_THREADS; i++)
	{
		data[i].id = i;
		data[i].x_start = 0;
		data[i].x_end = xsize;
		data[i].img_start = start;
		data[i].img_end = end;
		data[i].y_max = ysize;
		data[i].x_max = x_max;
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
		if (y_end > y_size)
		{
			y_end = y_size;
		}
		

		data[i].y_start = y_start;
		data[i].y_end = y_end;



		// Assign next chunk
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

int main (int argc, char ** argv) {
	int radius;
    int xsize, ysize, colmax;
    pixel src[MAX_PIXELS];
	pixel dst[MAX_PIXELS];
    struct timespec stime, etime;
	blur_data data[NUM_THREADS];
	#define MAX_RAD 1000

    double w[MAX_RAD];

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

    printf("Has read the image, generating coefficients\n");

    /* filter */
    get_gauss_weights(radius, w);

	printf("Setting up threads\n");

	calc_thread_data(data, xsize, ysize, radius, w, src);

    printf("Calling filter\n");

	clock_gettime(CLOCK_REALTIME, &stime);

	//Apply blur filter for each threads segment
	for (t = 0; t<NUM_THREADS; t++){
		printf("Initiating thread: %ld\n", t);
		ret = pthread_create(&threads[t], NULL, calc_average, (void*)&data[t]);
		if (ret){
			printf("Failed to initiate thread: %ld\n!", t);
			exit(-1);
		}
	}
	for (t = 0; t<NUM_THREADS; t++){
		pthread_join(threads[t], NULL);
	}



    //blurfilter(xsize, ysize, src, radius, w);

    clock_gettime(CLOCK_REALTIME, &etime);

    printf("Filtering took: %g secs\n", (etime.tv_sec  - stime.tv_sec) +
	   1e-9*(etime.tv_nsec  - stime.tv_nsec)) ;

    /* write result */
    printf("Writing output file\n");
    
    if(write_ppm (argv[3], xsize, ysize, (char *)src) != 0)
      exit(1);


    return(0);
}
