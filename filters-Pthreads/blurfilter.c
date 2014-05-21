/*
  File: blurfilter.c

  Implementation of blurfilter function.
    
 */
#include <stdio.h>
#include "blurfilter.h"
#include "ppmio.h"



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

  //printf("y_start %d, y_end %d, radius %d, img_start %d, img_end %d of thread %d\n", data->y_start, data->y_end, data->radius, data->img_start, data->img_end, data->id);

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



void blurfilter(const int xsize, const int ysize, pixel* src, const int radius, const double *w){
  int x,y,x2,y2, wi;
  double r,g,b,n, wc;
  pixel dst[MAX_PIXELS];


  for (y=0; y<ysize; y++) {
    for (x=0; x<xsize; x++) {
      r = w[0] * pix(src, x, y, xsize)->r;
      g = w[0] * pix(src, x, y, xsize)->g;
      b = w[0] * pix(src, x, y, xsize)->b;
      n = w[0];
      for ( wi=1; wi <= radius; wi++) {
	wc = w[wi];
	x2 = x - wi;
	if(x2 >= 0) {
	  r += wc * pix(src, x2, y, xsize)->r;
	  g += wc * pix(src, x2, y, xsize)->g;
	  b += wc * pix(src, x2, y, xsize)->b;
	  n += wc;
	}
	x2 = x + wi;
	if(x2 < xsize) {
	  r += wc * pix(src, x2, y, xsize)->r;
	  g += wc * pix(src, x2, y, xsize)->g;
	  b += wc * pix(src, x2, y, xsize)->b;
	  n += wc;
	}
      }
      pix(dst,x,y, xsize)->r = r/n;
      pix(dst,x,y, xsize)->g = g/n;
      pix(dst,x,y, xsize)->b = b/n;
    }
  }

  for (y=0; y<ysize; y++) {
    for (x=0; x<xsize; x++) {
      r = w[0] * pix(dst, x, y, xsize)->r;
      g = w[0] * pix(dst, x, y, xsize)->g;
      b = w[0] * pix(dst, x, y, xsize)->b;
      n = w[0];
      for ( wi=1; wi <= radius; wi++) {
	wc = w[wi];
	y2 = y - wi;
	if(y2 >= 0) {
	  r += wc * pix(dst, x, y2, xsize)->r;
	  g += wc * pix(dst, x, y2, xsize)->g;
	  b += wc * pix(dst, x, y2, xsize)->b;
	  n += wc;
	}
	y2 = y + wi;
	if(y2 < ysize) {
	  r += wc * pix(dst, x, y2, xsize)->r;
	  g += wc * pix(dst, x, y2, xsize)->g;
	  b += wc * pix(dst, x, y2, xsize)->b;
	  n += wc;
	}
      }
      pix(src,x,y, xsize)->r = r/n;
      pix(src,x,y, xsize)->g = g/n;
      pix(src,x,y, xsize)->b = b/n;
    }
  }

}



