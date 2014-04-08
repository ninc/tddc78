#include "thresfilter.h"


#define uint unsigned int 

void thresfilter(const int start, const int stop, const int threshold, pixel* src){

  uint i = 0;
  uint psum = 0;
  for(i = start; i < stop; i++) {
    psum = (uint)src[i].r + (uint)src[i].g + (uint)src[i].b;
    
    // If darker than average pixel color ---> Make pixel black
    if(threshold > psum) {
      src[i].r = src[i].g = src[i].b = 0;
    }
    // Brighter than average pixel color --> Make white
    else {
      src[i].r = src[i].g = src[i].b = 255;
    }
  }
}

// Calculates the summation of all pixel values from start to stop
int calcThreshold(const int start, const int stop, const pixel* src){
  uint sum = 0;
  uint i = 0;
  for(i = start; i < stop; i++) {
    sum += (uint)src[i].r + (uint)src[i].g + (uint)src[i].b;
  }

  return sum;
}
