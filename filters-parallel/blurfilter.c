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



void blur(const int xsize, const int ysize, pixel* src, pixel* dst, const int radius, const double *w, int displ, int displ_down, int recelm){
  
  int x,y,x2,y2, wi;
  double r,g,b,n, wc;
  int row, pos;
  int lowerbound, upperbound;
  int srcpos;

  pixel temp[recelm];

  for (y=0; y<ysize; y++) {
    for (x=0; x<xsize; x++) {
      y2 = y + radius;
      row = y2*xsize;
      pos = x + row;

      r = w[0] * src[pos].r;
      g = w[0] * src[pos].g;
      b = w[0] * src[pos].b;
      n = w[0];
      //start scanning radius of certain point in picture
      for ( wi=1; wi <= radius; wi++) {
	wc = w[wi];
	x2 = x - wi;
	if(x2 >= 0) {
	  row = y2*xsize;
	  pos = x2 + row;
	  //Calculate blur left of actual pixel
	  r += wc * src[pos].r;
	  g += wc * src[pos].g;
	  b += wc * src[pos].b;
	  n += wc;
	}
	x2 = x + wi;
	if(x2 < xsize) {
	  row = y2*xsize;
	  pos = x2 + row;
	  //Calculate blur right of actual pixel
	  r += wc * src[pos].r;
	  g += wc * src[pos].g;
	  b += wc * src[pos].b;
	  n += wc;
	}
      }

      row = y*xsize;
      pos = x + row;
      temp[pos].r = r/n;
      temp[pos].g = g/n;
      temp[pos].b = b/n;
      /*
      pix(dst,x,y, xsize)->r = r/n;
      pix(dst,x,y, xsize)->g = g/n;
      pix(dst,x,y, xsize)->b = b/n;
      */
    }
  }

  upperbound = displ - radius;
  lowerbound = displ_down - radius;
  /*
  //Calculate max upper radius
  if(radius > displ)
    upperbound = displ;
  else
    upperbound = radius;

  //Calculate max lower radius
  if(radius > displ_down)
    lowerbound = displ;
  else
    lowerbound = radius;

  for (y=0; y<ysize; y++) {
    for (x=0; x<xsize; x++) {
      y2 = y;
      row = y2*xsize;
      pos = x + row;

      r = w[0] * temp[pos].r;
      g = w[0] * temp[pos].g;
      b = w[0] * temp[pos].b;
      n = w[0];
      for ( wi=1; wi <= radius; wi++) {
	wc = w[wi];
	y2 = y - wi;
	if((upperbound - wi) >= 0) {
	  row = y2*xsize;
	  pos = x + row;
	  //Calculate blur left of actual pixel
	  r += wc * temp[pos].r;
	  g += wc * temp[pos].g;
	  b += wc * temp[pos].b;
	  n += wc;
	}
	y2 = y + wi;
	if((lowerbound - wi) >= 0) {

	  row = y2*xsize;
	  pos = x2 + row;
	  //Calculate blur right of actual pixel
	  r += wc * temp[pos].r;
	  g += wc * temp[pos].g;
	  b += wc * temp[pos].b;
	  n += wc;
	}
      }
      y2 = y + radius;
      row = y*xsize;
      pos = x + row;
      srcpos = y2*xsize + x;

      // Test
      /*
      dst[pos].r = 255;
      dst[pos].g = 0;
      dst[pos].b = 0;
      */

  //      pix(dst,x,y, xsize)->r = r/n;
  //    pix(dst,x,y, xsize)->g = g/n;
  //    pix(dst,x,y, xsize)->b = b/n;

      // Difficult displacements of positions
      //dst[pos].r = (src[srcpos].r ) /n;
      //dst[pos].g = (src[srcpos].g ) /n;
      //dst[pos].b = (src[srcpos].b ) /n;
      /*
      pix(dst,x,y, xsize)->r = r/n;
      pix(dst,x,y, xsize)->g = g/n;
      pix(dst,x,y, xsize)->b = b/n;
      */
    }
  }
  */
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
      //start scanning radius of certain point in picture
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



