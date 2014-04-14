#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ppmio.h"
#include "blurfilter.h"
#include "gaussw.h"
#include "mpi.h"


#define MAX_RAD 1000


pixel* pix_calc(pixel* image, const int xx, const int yy, const int xsize)
{
  register int off = xsize*yy + xx;

#ifdef DBG
  if(off >= MAX_PIXELS) {
    fprintf(stderr, "\n Terribly wrong: %d %d %d\n",xx,yy,xsize);
  }
#endif
  return (image + off);
}


void blur_calc(const int xsize, const int ysize, pixel* src, pixel* dst, const int radius, const double *w){
  int x,y,x2,y2, wi;
  double r,g,b,n, wc;

  for (y=radius; y<ysize-radius; y++) {
    for (x=0; x<xsize; x++) {
      r = w[0] * pix_calc(src, x, y, xsize)->r;
      g = w[0] * pix_calc(src, x, y, xsize)->g;
      b = w[0] * pix_calc(src, x, y, xsize)->b;
      n = w[0];
      //start scanning radius of certain point in picture
      for ( wi=1; wi <= radius; wi++) {
	wc = w[wi];
	x2 = x - wi;
	if(x2 >= 0) {
	  r += wc * pix_calc(src, x2, y, xsize)->r;
	  g += wc * pix_calc(src, x2, y, xsize)->g;
	  b += wc * pix_calc(src, x2, y, xsize)->b;
	  n += wc;
	}
	x2 = x + wi;
	if(x2 < xsize) {
	  r += wc * pix_calc(src, x2, y, xsize)->r;
	  g += wc * pix_calc(src, x2, y, xsize)->g;
	  b += wc * pix_calc(src, x2, y, xsize)->b;
	  n += wc;
	}
      }
      pix_calc(dst,x,y, xsize)->r = r/n;
      pix_calc(dst,x,y, xsize)->g = g/n;
      pix_calc(dst,x,y, xsize)->b = b/n;
    }
  }
}

void blur_calc_start(const int xsize, const int ysize, pixel* src, pixel* dst, const int radius, const double *w){
  int x,y,x2,y2, wi;
  double r,g,b,n, wc;

  for (y=0; y<ysize-radius; y++) {
    for (x=0; x<xsize; x++) {
      r = w[0] * pix_calc(src, x, y, xsize)->r;
      g = w[0] * pix_calc(src, x, y, xsize)->g;
      b = w[0] * pix_calc(src, x, y, xsize)->b;
      n = w[0];
      //start scanning radius of certain point in picture
      for ( wi=1; wi <= radius; wi++) {
	wc = w[wi];
	x2 = x - wi;
	if(x2 >= 0) {
	  r += wc * pix_calc(src, x2, y, xsize)->r;
	  g += wc * pix_calc(src, x2, y, xsize)->g;
	  b += wc * pix_calc(src, x2, y, xsize)->b;
	  n += wc;
	}
	x2 = x + wi;
	if(x2 < xsize) {
	  r += wc * pix_calc(src, x2, y, xsize)->r;
	  g += wc * pix_calc(src, x2, y, xsize)->g;
	  b += wc * pix_calc(src, x2, y, xsize)->b;
	  n += wc;
	}
      }
      pix_calc(dst,x,y, xsize)->r = r/n;
      pix_calc(dst,x,y, xsize)->g = g/n;
      pix_calc(dst,x,y, xsize)->b = b/n;
    }
  }
}

void blur_calc_last(const int xsize, const int ysize, pixel* src, pixel* dst, const int radius, const double *w){
  int x,y,x2,y2, wi;
  double r,g,b,n, wc;

  for (y=radius; y<ysize; y++) {
    for (x=0; x<xsize; x++) {
      r = w[0] * pix_calc(src, x, y, xsize)->r;
      g = w[0] * pix_calc(src, x, y, xsize)->g;
      b = w[0] * pix_calc(src, x, y, xsize)->b;
      n = w[0];
      //start scanning radius of certain point in picture
      for ( wi=1; wi <= radius; wi++) {
	wc = w[wi];
	x2 = x - wi;
	if(x2 >= 0) {
	  r += wc * pix_calc(src, x2, y, xsize)->r;
	  g += wc * pix_calc(src, x2, y, xsize)->g;
	  b += wc * pix_calc(src, x2, y, xsize)->b;
	  n += wc;
	}
	x2 = x + wi;
	if(x2 < xsize) {
	  r += wc * pix_calc(src, x2, y, xsize)->r;
	  g += wc * pix_calc(src, x2, y, xsize)->g;
	  b += wc * pix_calc(src, x2, y, xsize)->b;
	  n += wc;
	}
      }
      pix_calc(dst,x,y, xsize)->r = r/n;
      pix_calc(dst,x,y, xsize)->g = g/n;
      pix_calc(dst,x,y, xsize)->b = b/n;
    }
  }
}







void blur_start(const int xsize, const int ysize, pixel* src, pixel* dst,  const int radius, const double *w){

  int x,y,x2,y2, wi;
  double r,g,b,n, wc;

  for (y=0; y<ysize-radius; y++) {
    for (x=0; x<xsize; x++) {
      r = w[0] * pix_calc(dst, x, y, xsize)->r;
      g = w[0] * pix_calc(dst, x, y, xsize)->g;
      b = w[0] * pix_calc(dst, x, y, xsize)->b;
      n = w[0];
      for ( wi=1; wi <= radius; wi++) {
	wc = w[wi];
	y2 = y - wi;
	if(y2 >= 0) {
	  r += wc * pix_calc(dst, x, y2, xsize)->r;
	  g += wc * pix_calc(dst, x, y2, xsize)->g;
	  b += wc * pix_calc(dst, x, y2, xsize)->b;
	  n += wc;
	}
	y2 = y + wi;
	if(y2 < ysize) {
	  r += wc * pix_calc(dst, x, y2, xsize)->r;
	  g += wc * pix_calc(dst, x, y2, xsize)->g;
	  b += wc * pix_calc(dst, x, y2, xsize)->b;
	  n += wc;
	}
      }
      pix_calc(src,x,y, xsize)->r = r/n;
      pix_calc(src,x,y, xsize)->g = g/n;
      pix_calc(src,x,y, xsize)->b = b/n;
    }
  }

}


void blur_last(const int xsize, const int ysize, pixel* src, pixel* dst,  const int radius, const double *w){

  int x,y,x2,y2, wi;
  double r,g,b,n, wc;

  for (y=radius; y<ysize; y++) {
    for (x=0; x<xsize; x++) {
      r = w[0] * pix_calc(dst, x, y, xsize)->r;
      g = w[0] * pix_calc(dst, x, y, xsize)->g;
      b = w[0] * pix_calc(dst, x, y, xsize)->b;
      n = w[0];
      for ( wi=1; wi <= radius; wi++) {
	wc = w[wi];
	y2 = y - wi;
	if(y2 >= 0) {
	  r += wc * pix_calc(dst, x, y2, xsize)->r;
	  g += wc * pix_calc(dst, x, y2, xsize)->g;
	  b += wc * pix_calc(dst, x, y2, xsize)->b;
	  n += wc;
	}
	y2 = y + wi;
	if(y2 < ysize) {
	  r += wc * pix_calc(dst, x, y2, xsize)->r;
	  g += wc * pix_calc(dst, x, y2, xsize)->g;
	  b += wc * pix_calc(dst, x, y2, xsize)->b;
	  n += wc;
	}
      }
      pix_calc(src,x,y, xsize)->r = r/n;
      pix_calc(src,x,y, xsize)->g = g/n;
      pix_calc(src,x,y, xsize)->b = b/n;
    }
  }

}

void blur(const int xsize, const int ysize, pixel* src, pixel* dst,  const int radius, const double *w){

  int x,y,x2,y2, wi;
  double r,g,b,n, wc;

  for (y=0; y<ysize; y++) {
    for (x=0; x<xsize; x++) {
      r = w[0] * pix_calc(dst, x, y, xsize)->r;
      g = w[0] * pix_calc(dst, x, y, xsize)->g;
      b = w[0] * pix_calc(dst, x, y, xsize)->b;
      n = w[0];
      for ( wi=1; wi <= radius; wi++) {
	wc = w[wi];
	y2 = y - wi;
	if(y2 >= 0) {
	  r += wc * pix_calc(dst, x, y2, xsize)->r;
	  g += wc * pix_calc(dst, x, y2, xsize)->g;
	  b += wc * pix_calc(dst, x, y2, xsize)->b;
	  n += wc;
	}
	y2 = y + wi;
	if(y2 < ysize) {
	  r += wc * pix_calc(dst, x, y2, xsize)->r;
	  g += wc * pix_calc(dst, x, y2, xsize)->g;
	  b += wc * pix_calc(dst, x, y2, xsize)->b;
	  n += wc;
	}
      }
      pix_calc(src,x,y, xsize)->r = r/n;
      pix_calc(src,x,y, xsize)->g = g/n;
      pix_calc(src,x,y, xsize)->b = b/n;
    }
  }

}





int main (int argc, char ** argv) {
  int radius;
  int xsize, ysize, colmax;
  pixel src[MAX_PIXELS];
  pixel* rbuf = NULL;
  pixel* local_dst = NULL;
  struct timespec stime, etime;

  double w[MAX_RAD];

  //pixel dst[MAX_PIXELS];
  int rank, size;
  int root = 0;
  int y;
  

    MPI_Init(NULL, NULL);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

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

    //Global
    int displc[size];
    int sendcount[size];
    // Local variables
    int displ;
    int sendcnt;

    /* Take care of the arguments */

    if (argc != 4) {
      if(rank==root)
	fprintf(stderr, "Usage: %s radius infile outfile\n", argv[0]);
      MPI_Finalize();
      exit(1);
    }

    if(rank==root)
      {
	radius = atoi(argv[1]);
	if((radius > MAX_RAD) || (radius < 1)) {
	  fprintf(stderr, "Radius (%d) must be greater than zero and less then %d\n", radius, MAX_RAD);
	  MPI_Finalize();
	  exit(1);
	}

	/* read file */
	if(read_ppm (argv[2], &xsize, &ysize, &colmax, (char *) src) != 0){
	  MPI_Finalize();
	  exit(1);
	}

	if (colmax > 255) {
	  fprintf(stderr, "Too large maximum color-component value\n");
	  MPI_Finalize();
	  exit(1);
	}

	printf("Has read the image, generating coefficients\n");

	/* filter */
	get_gauss_weights(radius, w);

	printf("Calling filter\n");

	y = (ysize/size);
      }

    //Creating buffer
    
    MPI_Bcast(&radius, 1, MPI_INT, root, MPI_COMM_WORLD);
    
    MPI_Bcast(&w, MAX_RAD, MPI_DOUBLE, root, MPI_COMM_WORLD);
    
    MPI_Bcast(&xsize, 1, MPI_INT, root, MPI_COMM_WORLD);
    MPI_Bcast(&y, 1, MPI_INT, root, MPI_COMM_WORLD);


    
    // Calculate distribution of threads
    //Top layer
    if(rank == root)
      {
	displ = 0;
	sendcnt = (y+radius)*xsize; 
      }
    //Bottom layer
    else if(rank==size-1)
      {
	//Kanske behöver +1 
	displ = rank*(y+radius)*xsize;
	sendcnt = (y+radius)*xsize;
      }
    //Middle layers
    else
      {
	// Kanske behöver +1
	displ = rank*(y+radius)*xsize;
	//displc[rank] = (rank-1)*(y+radius)*xsize + (y+radius)*xsize;
	sendcnt = (y+2*radius)*xsize;
      }

    //printf("Local displacement for rank %u: %u\n", rank, displ);
    //printf("Send count for rank %u: %u\n", rank, sendcnt);


    //Create read buffer
    rbuf = (pixel *)malloc(sendcnt*sizeof(pixel)); 
    local_dst = (pixel *)malloc(y*xsize*sizeof(pixel));


    //Recieve global distribution of threads
    MPI_Gather(&displ, 1, MPI_INT, displc, 1, MPI_INT, root, MPI_COMM_WORLD);
    MPI_Gather(&sendcnt, 1, MPI_INT, sendcount, 1, MPI_INT, root, MPI_COMM_WORLD);


    // Where the action happens!
    clock_gettime(CLOCK_REALTIME, &stime);

    // Scatter the fucking whore bitch ass cunt
    MPI_Scatterv(src, sendcount, displc, mpi_img, rbuf, sendcnt, mpi_img, root, MPI_COMM_WORLD);
    //MPI_Scatter(dst, y*xsize, mpi_img, local_dst, y*xsize, mpi_img, root, MPI_COMM_WORLD);


    if(rank == root)
      {
	blur_calc_start(xsize, y, src, local_dst, radius, w);
	blur_start(xsize, y, src, local_dst, radius, w);
      }
    /*
    else if(rank == size - 1)
      {
	blur_calc_last(xsize, y, src, local_dst, radius, w);
	blur_last(xsize, y, src, local_dst, radius, w);
      }
    else
      {
	blur_calc(xsize, y, src, local_dst, radius, w);
	blur(xsize, y, src, local_dst, radius, w);
      }
    */
    /*

       int MPI_Gatherv(void *sendbuf, int sendcnt, MPI_Datatype sendtype,
                       void *recvbuf, int *recvcnts, int *displs,
                       MPI_Datatype recvtype, int root, MPI_Comm comm)

    */
    // Scatter the fucking whore bitch ass cunt
    //MPI_Gatherv(local_dst, y*xsize, mpi_img, src, sendcount, displc, mpi_img, root, MPI_COMM_WORLD);

    //MPI_Gather(local_dst, y*xsize, mpi_img, dst, y*xsize, mpi_img, root, MPI_COMM_WORLD);

    clock_gettime(CLOCK_REALTIME, &etime);

    if(rank==root)
      {
	printf("Filtering took: %g secs\n", (etime.tv_sec  - stime.tv_sec) +
	       1e-9*(etime.tv_nsec  - stime.tv_nsec)) ;

	/* write result */
	printf("Writing output file\n");
    
	if(write_ppm (argv[3], xsize, ysize, (char *)src) != 0){
	  MPI_Finalize();
	  exit(1);
	}
      }

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();
    return(0);
}
