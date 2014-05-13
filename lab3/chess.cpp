// my first program in C++
#include <iostream>
#include <stdio.h>


const int n  = 1000;
const int maxiter = 1000;
const double tol = 0.001;



//Set boundary conditions and initial values for the unknowns
void setBoundary(double[][]* T)
{
  int x_cord;
  int y_cord;

  for(x_cord = 0; x_cord<n+1; x_cord++)
    {
      for(y_cord = 0; y_cord<n+1; y_cord++)
	{
	  // Set the boundary values
	  if(y_cord == 0 || y_cord == n+1)
	    {
	      T[x_cord][y_cord] = 1.0f;
	    }
	  else if(x_cord == n+1)
	    {
	      T[x_cord][y_cord] = 2.0f;
	    }
	  else
	    {
	      T[x_cord][y_cord] = 0.0f;
	    }
	}
    }

  return 0;
}

//Solve the linear system of equations using the Jacobi method
void jacobi(double[][] *T)
{
  int i, j, k;
  double error = 0.0f;



  




}


int main (int argc, char ** argv) {
  
  double T[n][n]; // Kanske inte n+1 utan bara n
  float t1, t2;
  char str;

  setBoundary(&T);
  
  //Call CPU TIME
  jacobi(&T);
  //Call CPU TIME

}
