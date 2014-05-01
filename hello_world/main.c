/*                                                                              
 * Hello World in C
 *
 MPI:
 icc -Nmpi -o myapp myapp.c 
 salloc -N 1 -t 1 mpprun thres ../img/im1.ppm  ../outputimg/im1threstest.ppm
 
 
 OpenMP:
 icc -openmp -o main main.c
 export OMP_NUM_THREADS=4
 ompsalloc -N 1 -t 1 main
 

*/
#include <stdio.h>
#include "omp.h"

int main()
{
  printf("Hello from master thread");
#pragma omp parallel
  {
    printf("Hello from thread %d of %d\n", omp_get_thread_num(), omp_get_num_threads());
    
  }
}
