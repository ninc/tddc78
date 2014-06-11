// Mädz super duper particle simulation (MSDPS)
//icpc -Nmpi -o mainMPI mainMPI.cpp physics.c
//salloc -N 1 -t 1 mpprun ./mainMPI
//mpprun ./mainMPI

//#include "mpi.h"
#include <iostream>
#include "definitions.h"
#include "coordinate.h"
#include "physics.h"
#include <math.h>
#include <time.h>
#include <stdio.h>
#include "ll.h"


using namespace std;

#define max_vel 50
#define max_l 10000
#define n 10
#define t 50
#define root 0

// Calculate the pressure and verify that the calculations are correct
float calc_pressure(float *momentum)
{

  return *momentum /= (t*4*max_l);
}

//TODO
//Create a linked list interface

//Initiate the particles for each thread as a linked list

//Algorithm should detect if a particle is out of the assigned threads area
//then send it to that thread and delete it in its own list.
//How to handle walls (should one thread handle it)?


// Create individual particles with random position and speed
pcord_t* create_particle(int x, int y, int l)
{

  pcord_t* particle = (pcord_t *) malloc(sizeof(pcord_t));


  //Help variables
  float r;
  float phi;

  //initialize the particles
  double probability = rand() /(double)RAND_MAX;
  r= probability*max_vel;
  probability = rand() /(double)RAND_MAX;
  phi= probability*2*PI;
  // Set the particle speed
  particle->vx = r*cos(phi);
  particle->vy = r*sin(phi);

  //randomize the coordinates for the particle
  probability = rand() /(double)RAND_MAX;
  particle->x = x + probability* l;
  probability = rand() /(double)RAND_MAX;
  particle->y = y + probability* l;

  return particle;
}

// Initates particles inside an area
void init_particles(int x, int y, int l)
{
  linit();

  int i;
  pcord_t* temp;

  time_t tt1;
  time(&tt1);
  srand(tt1);

  // Fill the linked list with particles

  for(i = 0; i<n; i++)
    {
      if(i%1000 == 0)
	cout << "i=" << i << endl;

      //Create particle data
      temp = create_particle(x, y, l);
      
      //Add to linked list
      ladd_last(temp);
    }	
  return;
}


int main (int argc, char ** argv) {


  float momentum;
  clock_t t1,t2;
  cord_t wall;

  cout << "Starting initation" << endl;
  init_particles(max_l, max_l, max_l);
  cout << "Initation finished" << endl;
   
  p* temp = lget_first();

  lprint_id();

  cout << "removing middle item" << endl;
  lremove(temp->next);

  lprint_id();

  cout << "removing last item" << endl;
  lremove(lget_last());

  lprint_id();

  cout << "removing first item" << endl;
  lremove(temp);

  lprint_id();

  cout << "removing all items" << endl;
  lclear();

  lprint_id();

}
