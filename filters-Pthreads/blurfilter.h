/*
  File: blurfilter.h

  Declaration of pixel structure and blurfilter function.
    
 */

#ifndef _BLURFILTER_H_
#define _BLURFILTER_H_

#include <pthread.h>

pthread_barrier_t barr;

/* NOTE: This structure must not be padded! */
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


void blurfilter(const int xsize, const int ysize, pixel* src, const int radius, const double *w);

void* blur(void *d);

#endif
