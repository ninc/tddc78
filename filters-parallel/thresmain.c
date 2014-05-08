#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ppmio.h"
#include "thresfilter.h"

#include "mpi.h"

#define uint unsigned int 



void apply_threshold_filter(const int size, const int threshold, pixel* src)
{
  uint i, psum = 0;

  for(i = 0; i < size; i++) {
    psum = (uint)src[i].r + (uint)src[i].g + (uint)src[i].b;
    
    // If darker than average pixel color ---> Make pixel black
    if(threshold > psum) {
      src[i].r = 0;
      src[i].g = 0;
      src[i].b = 0;
    }
    // Brighter than average pixel color --> Make white
    else {
      src[i].r = 255;
      src[i].g = 255;
      src[i].b = 255;
    }
  }
}

uint sum_avg_pix(const int size, pixel* src){

  uint sum, i;

  for(i = 0, sum = 0; i < size; i++) {
    sum += (uint)src[i].r + (uint)src[i].g + (uint)src[i].b;
  }

  return sum/size;
}

int main (int argc, char ** argv) {
  
    int xsize, ysize, colmax;
    int chunk;
    pixel src[MAX_PIXELS];
    pixel* scatter_src = NULL;
    pixel* rbuf = NULL;
    pixel* sbuf = NULL;
    struct timespec stime, etime;
    int rank, size;
    uint *global_sum = NULL;
    uint *local_sum = NULL;
    uint avg = 0;
    int root = 0;
    uint real_avg = 0;
    uint imgsize;
    

    MPI_Init(NULL, NULL);
    /*
    MPI_Comm comm_world, comm_worker;
    MPI_Group group_world, group_worker;
    comm_world = MPI_COMM_WORLD;
    MPI_Comm_group(comm_world, &group_world);

    MPI_Group_excl(group_world, 1, 0, &group_worker);  // process 0 not member
    MPI_Comm_create(comm_world, group_worker, &comm_worker);

    */
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* Take care of the arguments */

    /* create a type for struct car */
    const int nitems=3;
    int blocklengths[3] = {1,1,1};

    MPI_Datatype types[3] = {MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR};
    // MPI_Datatype types[3] = {MPI_CHAR, MPI_CHAR, MPI_CHAR};
    MPI_Datatype mpi_img;
    MPI_Aint offsets[3];
    
    offsets[0] = offsetof(pixel, r);
    offsets[1] = offsetof(pixel, g);
    offsets[2] = offsetof(pixel, b);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_img);
    MPI_Type_commit(&mpi_img);

    
    if (argc != 3) {
      fprintf(stderr, "Usage: %s infile outfile\n", argv[0]);
      MPI_Finalize();
      exit(1);
    }
    
    
    if(rank==root) {
      /* read file */
      if(read_ppm (argv[1], &xsize, &ysize, &colmax, (char *) src) != 0)
	{
	  MPI_Finalize();
	  exit(2);
	}
      if (colmax > 255) {
	fprintf(stderr, "Too large maximum color-component value\n");
	MPI_Finalize();
	exit(3);
      }
      
      
      printf("Has read the image, calling filter\n");
      imgsize = xsize*ysize;
      real_avg = sum_avg_pix(imgsize, src);
    }

    clock_gettime(CLOCK_REALTIME, &stime);


    // Calc and broadcast chunks
    if(rank==root)
      {
      //Calc chunks
      chunk = imgsize/size;
      global_sum = (uint *)malloc(sizeof(uint));
      *global_sum = 0;
      scatter_src = src;
      }

    MPI_Bcast(&chunk, 1, MPI_UNSIGNED, root, MPI_COMM_WORLD);

    if(rank==root)
      {
	printf("Has broadcasted chunks. Calling Scatter\n");
	uint rest = chunk*(size) - imgsize;
	uint new_chunk = chunk;
	chunk = new_chunk;
      }

    //Create local buffer
    rbuf = (pixel *)malloc(chunk*sizeof(pixel)); 

    MPI_Scatter(src, chunk, mpi_img, rbuf, chunk, mpi_img, root, MPI_COMM_WORLD);
    //printf("chunk, %d \n", chunk);
    //Create local buffer
    local_sum = (uint *)malloc(sizeof(uint));
    *local_sum = sum_avg_pix(chunk, rbuf);
    //printf("local sum: %d \n", *local_sum);

    MPI_Reduce(local_sum, global_sum, 1, MPI_UNSIGNED, MPI_SUM, root, MPI_COMM_WORLD);
    
    if(rank==root)
      {
	//printf("Global_sum: %d\n", *global_sum);
	avg = *global_sum / size;
	printf("Avg: %d\nRealAvg: %d\n", avg, real_avg);
      }
    
    MPI_Bcast(&avg, 1, MPI_UNSIGNED, root, MPI_COMM_WORLD);

    // Scatter again - Apply filtering
    //Not needed
    //MPI_Scatter(src, chunk, mpi_img, rbuf, chunk, mpi_img, root, MPI_COMM_WORLD);
    
    apply_threshold_filter(chunk, avg, rbuf);


    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Gather(rbuf, chunk, mpi_img, src, chunk, mpi_img, root, MPI_COMM_WORLD);
    
    clock_gettime(CLOCK_REALTIME, &etime);

    if(rank==0)
      {
	printf("Filtering took: %g secs\n", (etime.tv_sec  - stime.tv_sec) +
	       1e-9*(etime.tv_nsec  - stime.tv_nsec)) ;

	/* write result */
	printf("Writing output file\n");
    
	real_avg = sum_avg_pix(imgsize, src);
	printf("RealAvg: %d\n", real_avg);

	if(write_ppm (argv[2], xsize, ysize, (char *)src) != 0)
	  {
	    MPI_Finalize();
	    exit(4);
	  }
	free(sbuf);
	free(global_sum);
      }

    free(local_sum);
    free(rbuf);

    MPI_Finalize();
    return(0);
}
