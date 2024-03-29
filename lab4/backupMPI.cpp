// Mädz super duper particle simulation (MSDPS)
//icpc -Nmpi -o main mainMPI.cpp physics.c
//mpprun ./main

#include "mpi.h"
#include <iostream>
#include "definitions.h"
#include "coordinate.h"
#include "physics.h"
#include <math.h>
#include <time.h>

using namespace std;

#define max_vel 50
#define max_l 10000
#define part_sum 10000
#define t 50
#define root 0

//Setup needed data structes for the simulation
cord_t init(particle_t *particles)
{
  //allocate memory
  float r;
  float phi;

  time_t t1;
  time(&t1);
  srand(t1);
 
  //initialize the walls
  cord_t wall;
  wall.x0 = 0;
  wall.y0 = 0;
  wall.y1 = max_l;
  wall.x1 = max_l;

  //initialize the particles
  double probability = 0;
  for (int i = 0; i < n; i++){
    //randomize the velocity
    probability = rand() /(double)RAND_MAX;
    r= probability*max_vel;
    probability = rand() /(double)RAND_MAX;
    phi= probability*2*PI;
    particles[i].pcord.vx = r*cos(phi);
    particles[i].pcord.vy = r*sin(phi);
    //randomize the coordinates for the particles
    probability = rand() /(double)RAND_MAX;
    particles[i].pcord.x = probability* max_l;
    probability = rand() /(double)RAND_MAX;
    particles[i].pcord.y = probability* max_l;
  }
  return wall;
}

// Simulate particle pressure
int simulate(particle_t particles[n], cord_t wall, int size, int rank, MPI_Datatype particle_mpi)
{
  //begin simulation
  float collision = 0;
  float global_momentum = 0;
  float local_momentum = 0;
  int chunk = n/size;
  particle_t* rbuf = NULL;
  if(rank ==size-1){
    chunk = chunk + (n - chunk*size);
  }
  
  rbuf = (particle_t *)malloc(chunk*sizeof(particle_t)); 
  for(int i=0;i<t;i++){
    
    //Broadcast particles (read-only)
    MPI_Bcast(&particles, 1, particle_mpi, root, MPI_COMM_WORLD);
    //Send segment of particles to process
    MPI_Scatter(particles, chunk, particle_mpi, rbuf, chunk, particle_mpi, root, MPI_COMM_WORLD);
    //main loop
    for(int j=0;j<(chunk-1);j++){
      for(int k=0;k<n;k++){
	//Check for collisions
	collision = collide(&rbuf[j].pcord, &particles[k].pcord);
	//if collision
	if(collision != -1){
	  interact(&rbuf[j].pcord, &particles[k].pcord, collision);
	  break;
	}
      }
      
      //if no collision
      if(collision == -1) {
	//Move particles that have not collided
        feuler(&rbuf[j].pcord, i);
      }
      local_momentum += wall_collide(&rbuf[j].pcord, wall);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Gather(rbuf, chunk, particle_mpi, particles, chunk, particle_mpi, root, MPI_COMM_WORLD);

    // Communicate if needed bastard!
    
  }
  return global_momentum;
}

// Calculate the pressure and verify that the calculations are correct
float calc_pressure(float *momentum)
{

  return *momentum /= (t*4*max_l);
}

int main (int argc, char ** argv) {
  MPI_Init(NULL, NULL);
  MPI_Datatype pcord_t_mpi;
  MPI_Datatype particle_mpi;
  MPI_Datatype wall_mpi;
   
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  unsigned int n = size*part_sum;

  particle_t particles[n];
  float momentum;
  clock_t t1,t2;
  int rank;
  int size;
  cord_t wall;

  //---------------------------------------------------------------------------------
  //Create datatype for pcord in MPI
  const int nitems=4;
  int blocklengths[4] = {1,1,1, 1};
  MPI_Datatype types[4] = {MPI_FLOAT, MPI_FLOAT , MPI_FLOAT, MPI_FLOAT};
  //MPI_Datatype pcord_t_mpi;
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

   //Create datatype for walls in MPI
  const int nitems3=4;
  int blocklengths3[4] = {1,1,1,1};
  MPI_Datatype types3[4] = {MPI_FLOAT, MPI_FLOAT , MPI_FLOAT, MPI_FLOAT};
  //MPI_Datatype wall_mpi;
  MPI_Aint offsets3[4];
  offsets3[0] = offsetof(cord_t, x0);
  offsets3[1] = offsetof(cord_t, x1);
  offsets3[2] = offsetof(cord_t, y0);
  offsets3[3] = offsetof(cord_t, y1);
  MPI_Type_create_struct(nitems3, blocklengths3, offsets3, types3, &wall_mpi);
  MPI_Type_commit(&wall_mpi);
  //----------------------------------------------------------------------------------
  
  

  if(rank == root){
    cout<< "begin init particles" << endl;
    // Initialize simulation
   
    wall = init(particles);
    cout<< "finished init particles" << endl;
    t1 = clock();
  }
  //Bcast wall?'
  MPI_Bcast(&wall, 1, wall_mpi, root, MPI_COMM_WORLD);
  MPI_Bcast(&particles, 1, particle_mpi, root, MPI_COMM_WORLD);
  //cout<< "rank : " << rank <<  "particles coord x: " <<   particles->pcord.x <<  endl;

  //----------------------------------------------------------------------------------------------------------
  //begin simulation
  float collision = 0;
  float *global_momentum = NULL;
  float *local_momentum = NULL;
  int chunk = n/size;
  particle_t* rbuf = NULL;
  if(rank ==size-1){
    chunk = chunk + (n - chunk*size);
  }
  local_momentum = (float *)malloc(sizeof(float));
  *local_momentum = 0;
  if(rank == root){
    global_momentum = (float *)malloc(sizeof(float));
    *global_momentum = 0;
  }

  rbuf = (particle_t *)malloc(chunk*sizeof(particle_t)); 
  for(int i=0;i<t;i++){
    
    //Broadcast particles (read-only)
    MPI_Bcast(&particles, 1, particle_mpi, root, MPI_COMM_WORLD);
    //Send segment of particles to process
    MPI_Scatter(particles, chunk, particle_mpi, rbuf, chunk, particle_mpi, root, MPI_COMM_WORLD);
    //main loop
    for(int j=0;j<(chunk-1);j++){
      for(int k=0;k<n;k++){
	//Check for collisions
	collision = collide(&rbuf[j].pcord, &particles[k].pcord);
	//if collision
	if(collision != -1){
	  interact(&rbuf[j].pcord, &particles[k].pcord, collision);
	  break;
	}
      }
      //if no collision
      if(collision == -1) {
	//Move particles that have not collided
        feuler(&rbuf[j].pcord, i);
      }
      *local_momentum += wall_collide(&rbuf[j].pcord, wall);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Gather(rbuf, chunk, particle_mpi, particles, chunk, particle_mpi, root, MPI_COMM_WORLD);
  }
  MPI_Reduce(local_momentum, global_momentum, 1, MPI_FLOAT, MPI_SUM, root, MPI_COMM_WORLD);

  //---------------------------------------------------------------------------------------------------------
  //simulate(particles, wall, size, rank, particle_mpi);

  //calculate the total pressure of the bag-in-box
  if(rank == root){
    t2=clock();
    cout<< "pressure: " <<   calc_pressure(global_momentum) <<  endl;
    float sim_time = ((float)t2-(float)t1)/CLOCKS_PER_SEC;
    cout<< "Simulation finished after: " <<   sim_time <<  endl;
  }
  MPI_Finalize();
  return 0;
}
