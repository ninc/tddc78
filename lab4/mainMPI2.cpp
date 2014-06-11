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
#define n 10000
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
      //Create particle data
      temp = create_particle(x, y, l);
      
      //Add to linked list
      ladd_last(temp);
    }	
  return;
}


int main (int argc, char ** argv) {

  MPI_Init(NULL, NULL);
  MPI_Datatype pcord_t_mpi;
  MPI_Datatype particle_mpi;
  MPI_Datatype wall_mpi;
  int rank;
  int size;
   
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  float momentum = 0;
  float collision = 0;
  clock_t t1,t2;
  cord_t wall;

  //setup particles for each thread
  init_particles(max_l, max_l, max_l);

  //initialize the walls
  wall.x0 = 0;
  wall.y0 = 0;
  wall.y1 = max_l;
  wall.x1 = max_l;

  p* p1 = NULL;
  p* p2 = NULL;

  //---------------------------------------------------------------------------------
  //Create datatype for pcord in MPI
  const int nitems=4;
  int blocklengths[4] = {1,1,1, 1};
  MPI_Datatype types[4] = {MPI_FLOAT, MPI_FLOAT , MPI_FLOAT, MPI_FLOAT};
  MPI_Aint offsets[4];
  offsets[0] = offsetof(pcord_t, x);
  offsets[1] = offsetof(pcord_t, y);
  offsets[2] = offsetof(pcord_t, vx);
  offsets[3] = offsetof(pcord_t, vy);
 
  MPI_Type_create_struct(nitems, blocklengths, offsets, types, &pcord_t_mpi);
  MPI_Type_commit(&pcord_t_mpi);
  //Create datatype for particle in MPI
  const int nitems2=2;
  int blocklengths2[2] = {1,1};
  MPI_Datatype types2[2] = {pcord_t_mpi, MPI_INT};
  //MPI_Datatype particle_mpi;
  MPI_Aint offsets2[2];
  offsets2[0] = offsetof(particle_t, pcord);
  offsets2[1] = offsetof(particle_t, ptype);

  MPI_Type_create_struct(nitems2, blocklengths2, offsets2, types2, &particle_mpi);
  MPI_Type_commit(&particle_mpi);
  //----------------------------------------------------------------------------------
  





  //START THE TIMER
  t1 = clock();


  // Time step loop
  for(int i=0;i<t;i++){
    p1 = lget_first();


    //main loop
    for(int j=0;j<lget_size()-1;j++){
      p2 = p1->next;



      for(int k=j;k<lget_size();k++){
      
	//If outside of our area communicate it to other mpi processes

	//Check for collisions
	collision = collide(p1->pcord, p2->pcord);
	//if collision
	if(collision != -1){
	  interact(p1->pcord, p2->pcord, collision);
	  break;
	}
      }
      //if no collision
      if(collision == -1) {
	//Move particles that have not collided
	feuler(p1->pcord, i);
      }

      // Wall collisions
      momentum += wall_collide(p1->pcord, wall);
    
      p1 = p1->next;
    }
  }


  t2=clock();
  cout<< "pressure: " <<   calc_pressure(&momentum) <<  endl;
  float sim_time = ((float)t2-(float)t1)/CLOCKS_PER_SEC;
  cout<< "Simulation finished after: " <<   sim_time <<  endl;

  //Free memory
  lclear();

}
