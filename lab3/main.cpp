// my first program in C++
//export OMP_NUM_THREADS=16
//$ompsalloc ./main
// icpc -xHost -openmp -o main main.cpp 
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <vector>
#include "omp.h"

const int n  = 1000;
const int maxiter = 1000;
const double tol = 0.001;

using namespace std;

//Set boundary conditions and initial values for the unknowns
int setBoundary(double **T)
{
  int x;
  int y;

  for(y = 0; y<n; y++)
    {
      for(x = 0; x<n; x++)
	{
	  //cout << "x, y: " << x  << ", " << y << endl;
	  // Set the boundary values
	  if(y == 0 || y == n)
	    {
	      T[x][y] = 1.0f;
	    }
	  else if(x == n)
	    {
	      T[x][y] = 2.0f;
	    }
	  else
	    {
	      T[x][y] = 0.0f;
	    }
	}
    }

  return 0;
}

//Solve the linear system of equations using the Jacobi method
int jacobi(double **T)
{
  int k = 0;
  int x = 0;
  int y = 0;
  double t1, t2;
  int rank;
  int threads;
  double old_value = 0.0f;
  double global_error = 0.0f;
  double local_error = 0.0f;
  double tmp_error = 0.0f;

  t1 = omp_get_wtime();


  for(k =  1; k<maxiter; k++)
    {

      global_error = 0.0f;

#pragma omp parallel shared(global_error, T, k) private(local_error, tmp_error, old_value, y, x, rank, threads)
      {
	local_error = 0.0f;
	rank = omp_get_thread_num();
	threads = omp_get_num_threads();
	

	#pragma omp for 
	//reduction(max: global_error)
	//Black chess squares
	for(y = 1; y<n-1; y++)
	  {
	    //Diagonal correction
	    if(y%2)
	      {
		x = 1;
	      }
	    else
	      {
		x = 2;
	      }
	    
	    for(; x<n-1; x+=2)
	      {
		old_value = T[x][y];
		T[x][y] = (T[x-1][y] + T[x+1][y] + T[x][y-1] + T[x][y+1]) / 4.0;

		tmp_error = fabs(old_value - T[x][y]);

		// Error estimation
		if(local_error < tmp_error)
		  {
		    local_error = tmp_error;
		  }
	      }
	  }

	
 	#pragma omp barrier

	#pragma omp for reduction(max: global_error)
	//White chess squares
	for(y = 1; y<n-1; y++)
	  {
	    //Diagonal correction
	    if(y%2)
	      {
		x = 2;
	      }
	    else
	      {
		x = 1;
	      }
	    
	    for(; x<n-1; x+=2)
	      {
		old_value = T[x][y];
		T[x][y] = (T[x-1][y] + T[x+1][y] + T[x][y-1] + T[x][y+1]) / 4.0;

		tmp_error = fabs(old_value - T[x][y]);
		// Error estimation
		if(local_error < tmp_error)
		  {
		    local_error = tmp_error;
		  }
	      }
	    
	    // Reduce the amount of updates on shared variable
	    if(global_error < local_error)
	      {
		global_error = local_error;
	      }
	    
	  }
      }

      if(global_error<tol)
	{
	  break;
	}
 
    }


  t2 = omp_get_wtime();
  
  cout << "Error estimate: " << global_error << endl;
  cout << "Number of iterations: " << k << endl;
  cout << "Temperature of element T(1,1): " << T[1][1] << endl;
  cout << "Time: " << t2-t1 << endl;

  return 0;
}

// Allocates memory
double ** initMatrix()
{
  double **T;

  // Allocate memory
  T = new double*[n];
  for (int i = 0; i < n; ++i)
    T[i] = new double[n];

  return T;
}


// Cleans the memory
bool cleanupMatrix(double **T)
{
  
  // De-Allocate memory to prevent memory leak
  for (int i = 0; i < n; ++i)
    delete [] T[i];
  delete [] T;

  return true;
}


int main (int argc, char ** argv) {

  cout << "Initiating matrix" << endl;
  double **T = initMatrix();

  cout << "Initiating boundries" << endl;

  setBoundary(T);
  
  cout << "Boundaries set, calling Jacobi" << endl;

  jacobi(T);

  cout << "Cleaning up" << endl;
  cleanupMatrix(T);

  return 0;
}
