/*
  File: thresfilter.h

  Declaration of pixel structure and thresfilter function.
    
 */
#ifndef _THRESFILTER_H_
#define _THRESFILTER_H_
/* NOTE: This structure must not be padded! */
typedef struct _pixel {
    unsigned char r,g,b;
} pixel;

//void thresfilter(const int start, const int stop, const int threshold, pixel* src);
void thresfilter(const int start, const int stop, pixel* src);
int calcThreshold(const int start, const int stop, const pixel* src);


#endif
