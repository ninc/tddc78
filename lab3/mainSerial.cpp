// my first program in C++


// icpc -xHost -openmp -o main main.cpp 
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <vector>

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
  double old_value = 0.0f;
  double error = 0.0f;
  double local_error = 0.0f;
  double tmp_error = 0.0f;


  for(k =  0; k<maxiter; k++)
    {

      error = 0.0f;

      
      //cout << "Starting iter: " << k << endl;

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

	      //cout << "Black coords: " << x << ", " << y << endl;
	      old_value = T[x][y];
	      //cout << "Old_value: " << old_value << endl;
	      T[x][y] = (T[x-1][y] + T[x+1][y] + T[x][y-1] + T[x][y+1]) / 4.0;

	      tmp_error = fabs(old_value - T[x][y]);
	      //cout << "tmp_error: " << tmp_error << endl;
	      // Error estimation
	      if(error < tmp_error)
		{
		  //cout << "tmp_error: " << tmp_error << endl;
		  error = tmp_error;
		}
	    }
	}
      
      //cout << "Ending iter: " << k << endl;


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

	      //cout << "White coords: " << x << ", " << y << endl;
	      old_value = T[x][y];
	      //cout << "Old_value: " << old_value << endl;
	      T[x][y] = (T[x-1][y] + T[x+1][y] + T[x][y-1] + T[x][y+1]) / 4.0;

	      tmp_error = fabs(old_value - T[x][y]);
	      //cout << "tmp_error: " << tmp_error << endl;
	      // Error estimation
	      if(error < tmp_error)
		{
		  //cout << "tmp_error: " << tmp_error << endl;
		  error = tmp_error;
		}
	    }
	}

      

      if(error<tol)
	{
	  break;
	}

      
    }

  
  cout << "Error estimate: " << error << endl;
  cout << "Number of iterations: " << k << endl;
  cout << "Temperature of element T(1,1): " << T[1][1] << endl;


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
  
  //double T[n][n]; // Kanske inte n+1 utan bara n
  float t1, t2;
  char str;

  cout << "Initiating matrix" << endl;
  double **T = initMatrix();

  cout << "Initiating boundries" << endl;

  setBoundary(T);
  
  cout << "Boundaries set, calling Jacobi" << endl;

  //Call CPU TIME
  jacobi(T);
  //Call CPU TIME

  cout << "Time: " << t1-t0 << endl;

  cout << "Cleaning up" << endl;
  cleanupMatrix(T);

  return 0;
}
