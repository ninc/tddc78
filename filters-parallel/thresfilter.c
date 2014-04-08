#include "thresfilter.h"

void thresfilter(const int xsize, const int ysize, pixel* src){
#define uint unsigned int 

  uint sum, i, psum, nump;

  nump = xsize * ysize; //Area

  for(i = 0, sum = 0; i < nump; i++) {
    sum += (uint)src[i].r + (uint)src[i].g + (uint)src[i].b;
  }

  // sum = sum / nump;
  sum /= nump; // Average pixel color

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
}
