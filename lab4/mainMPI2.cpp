// Mädz super duper particle simulation (MSDPS)
//icpc -Nmpi -o mainMPI2 mainMPI2.cpp physics.c ll.cpp
//salloc -N 1 -t 1 mpprun ./mainMPI
//mpprun ./mainMPI

#include "mpi.h"

#include <iostream>
#include "definitions.h"
#include "coordinate.h"
#include "physics.h"
#include <math.h>
#include <time.h>
#include <stdio.h>
#include "ll.h"

#include <VT.h>


using namespace std;

#define max_vel 50
#define max_l 10000
#define n 10000
#define t 1000
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
pcord_t* create_particle(int x_start, int y_start, int x_end, int y_end)
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
  particle->x = x_start + probability*x_end;
  probability = rand() /(double)RAND_MAX;
  particle->y = y_start + probability*y_end;

  return particle;
}

// Initates particles inside an area
void init_particles(int x_start, int y_start, int x_end, int y_end, ll* particles)
{
  int i;
  pcord_t* temp;

  time_t tt1;
  time(&tt1);
  srand(tt1);

  // Fill the linked list with particles
  for(i = 0; i<n; i++)
    {
      //Create particle data
      temp = create_particle(x_start, y_start, x_end, y_end);
      
      //Add to linked list
      particles->ladd_last(temp);
    }	
  return;
}


//Add particles from buffer to local linked list
void add_particles(pcord_t * recv_north, pcord_t * recv_south, ll* particles){
  
  //north
  for(int i=0; i<10;i++){
    if(&recv_north[i] == NULL){
      break;
    }else{
      particles->ladd_last(&recv_north[i]);
      //recv_north[i]=NULL;
    }
  }
  //south
  for(int i=0; i<10;i++){
    if(&recv_south[i] == NULL){
      break;
    }else{
      particles->ladd_last(&recv_south[i]);
      //recv_south[i]=NULL;
    }
  }
  

}


//Prepare data for sending
void fillBuffer(pcord_t send_buffer[], int buffer_size, ll* ll)
{

  p* p;

  for(int i = 0; i<buffer_size; i++)
    {

      p = ll->lget_first();

      send_buffer[i] = *p->pcord;
   
      ll->lremove(p);

    }
}


int main (int argc, char ** argv) {


  //************************************* INITIATION *******************************


  MPI_Init(NULL, NULL);
  /* Tracing stuff for itac */
  int counter_class, sent_particles_handle, pressure_handle;
  long long int sent_particles;
  long long int range[0, 100000];
  double ranged[0, 100];
  VT_classdef("Counter class", &counter_class);
  VT_countdef ("sent_particles", counter_class, VT_COUNT_INTEGER64, VT_GROUP_PROCESS, range, "sent particles", &sent_particles_handle);
  VT_countdef ("pressure", counter_class, VT_COUNT_FLOAT, VT_GROUP_PROCESS, ranged, "pressure", &pressure_handle);
  VT_Region main_reg("main_reg", "main program");
  MPI_Datatype pcord_t_mpi;
  MPI_Datatype particle_mpi;
  MPI_Datatype wall_mpi;
  int rank;
  int size;
  ll* particles = new ll();
  ll* particles_south = new ll();
  ll* particles_north = new ll();
  int send_south_buff_size;
  int send_north_buff_size;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Request r_send_north, r_send_south;
  MPI_Request r_recv_north, r_recv_south;
  MPI_Status status;

  float* local_momentum = (float *)malloc(sizeof(float));
  *local_momentum = 0;
  float* global_momentum = (float *)malloc(sizeof(float));
  *global_momentum = 0;
  float collision = 0;
  clock_t t1,t2;
  cord_t wall;
  int x_start;
  int x_end;
  int y_start;
  int y_end;
  int north_neighbor;
  int south_neighbor;
  int chunk;
  
  // Calculate neighbours and local area
  north_neighbor = rank - 1;
  south_neighbor = rank + 1;
  chunk = max_l/size;
  
  x_start = 0;
  x_end = max_l;
  y_start = chunk*rank;
  y_end = y_start + chunk;

  // Error handling for neighbors
  if(rank == size -1)
    {
      south_neighbor = -1;
      y_end = max_l;
    }

  //setup particles for each thread
  init_particles(x_start, y_start, x_end, y_end, particles);

  //initialize the walls
  wall.x0 = 0;
  wall.y0 = 0;
  wall.y1 = max_l;
  wall.x1 = max_l;

  p* p1 = NULL;
  p* p2 = NULL;

  //initiate buffers
  pcord_t send_south[n];
  pcord_t send_north[n];
  pcord_t recv_south[n];
  pcord_t recv_north[n];
  int recv_south_buff_size;
  int recv_north_buff_size;
  int tag = 0;
  int tag2 = 1;

  /***************************** TEST ********************************************

  int test_size = 1;
  int test_recv_size;
  pcord_t send_test[1];
  pcord_t recv_test[1];
  pcord_t test_p;
  test_p.x = 11.0;
  send_test[0] = test_p;

  if(rank == root)
    {
      MPI_Issend(&test_size, 1, MPI_INT, south_neighbor, tag, MPI_COMM_WORLD, &r_send_south);

      //Send message
      MPI_Issend(&send_test, test_size*sizeof(pcord_t), MPI_CHAR, south_neighbor, tag2, 
		     MPI_COMM_WORLD, &r_send_south);
      cout << "Thread: " << rank << " Sent south:\n";	 
    }

  MPI_Barrier(MPI_COMM_WORLD);

  if(rank == root+1)
    {

      //Recieve number of particles
      MPI_Recv(&test_recv_size, 1, MPI_INT, north_neighbor, tag, MPI_COMM_WORLD, &status);
	
      // If we should recieve particles
      if(test_recv_size > 0)
	{
	  cout << "Thread: " << rank << " Recieved from north: " << test_recv_size << "\n";
	  //Recieve message
	  MPI_Recv(&recv_test, test_recv_size*sizeof(pcord_t), MPI_CHAR, north_neighbor,
		   tag2, MPI_COMM_WORLD, &status);
	  cout << "Thread: " << rank << " Got particle from north " << recv_test[0].x << endl;
	  
	}
    }

    //**********************************************************************/
 
  if(rank == root)
    {
      cout << "Starting calculations" << endl;
      //START THE TIMER
      t1 = clock();
    }

  // **************************** SIMULATION *******************************

  // Time step loop
  for(int i=0;i<t;i++){

    p1 = particles->lget_first();

    // Prepare to send old particles
    send_south_buff_size = particles_south->lget_size();
    send_north_buff_size = particles_north->lget_size();
    recv_south_buff_size = -1;
    recv_north_buff_size = -1;

    //Prepare data for sending
    fillBuffer(send_south, send_south_buff_size, particles_south);
    fillBuffer(send_north, send_north_buff_size, particles_north);


    // *********************************** COMMUNICATION ************************************
    VT_Region com_reg("com_reg", "Communication region");
    
    //Send north
    if(north_neighbor >= 0)
      {
	// Send data buffer size to neighbor
	MPI_Issend(&send_north_buff_size, 1, MPI_INT, north_neighbor, tag, MPI_COMM_WORLD, &r_send_north);

      if(send_north_buff_size > 0)
	{
	  //Send message
	  sent_particles += send_north_buff_size;
	  MPI_Issend(&send_north, send_north_buff_size*sizeof(pcord_t), MPI_CHAR, north_neighbor, tag2, 
		     MPI_COMM_WORLD, &r_send_north);
	}
      }


    // Send south
    if(south_neighbor >= 0)
      {
	// Send data buffer size to neighbor
	MPI_Issend(&send_south_buff_size, 1, MPI_INT, south_neighbor, tag, MPI_COMM_WORLD, &r_send_south);

	if(send_south_buff_size > 0)
	  {
	    //Send message
	     sent_particles += send_south_buff_size;
	    MPI_Issend(&send_south, send_south_buff_size*sizeof(pcord_t), MPI_CHAR, south_neighbor, tag2, 
		       MPI_COMM_WORLD, &r_send_south);
	  }

      }

    //Recieve from north
    if(north_neighbor >= 0)
      {
	//Recieve number of particles
	MPI_Recv(&recv_north_buff_size, 1, MPI_INT, north_neighbor, tag, MPI_COMM_WORLD, &status);
	
	// If we should recieve particles
	if(recv_north_buff_size > 0)
	  {
	    //Recieve message
	    MPI_Recv(&recv_north, recv_north_buff_size*sizeof(pcord_t), MPI_CHAR, north_neighbor,
		     tag2, MPI_COMM_WORLD, &status);
	  }


      }

    //Recieve from south
    if(south_neighbor >= 0)
      {
	//Recieve number of particles
	MPI_Recv(&recv_south_buff_size, 1, MPI_INT, south_neighbor, tag, MPI_COMM_WORLD, &status);

	// If we should recieve particles
	if(recv_south_buff_size > 0)
	  {
	    //Recieve message
	    MPI_Recv(&recv_south, recv_south_buff_size*sizeof(pcord_t), MPI_CHAR, south_neighbor,
		     tag2, MPI_COMM_WORLD, &status);
	  }
      }
    com_reg.end();
    // *********************************** SIMULATION LOOP ********************


    //main loop
    VT_Region coll_reg("coll_reg", "Collision detection");
    for(int j=0;j<particles->lget_size()-1;j++){
      p2 = p1->next;
      collision = 0;      
      //If outside of our area communicate it to other mpi processes
      // Particle is south of our area
      if(p1->pcord->y > y_end)
	{
	  // If thread have a south neighbor
	  if(south_neighbor != -1)
	    {

	      particles_south->ladd_last(p1->pcord);
	      p1 = p1->next;
	      // Remove the element
	      particles->lremove(p1->prev);
	    }
	  // If we don't have a neighbor south of us
	  else
	    {
	      //Collide with wall 
	      *local_momentum += wall_collide(p1->pcord, wall);
	      p1 = p1->next;
	    }
	}
      //Particle is north of our area
      else if(p1->pcord->y < y_start)
	{
	  //If we have a north neighbor
	  if(north_neighbor != -1)
	    {

	      particles_north->ladd_last(p1->pcord);
	      p1 = p1->next;
	      // Remove the element
	      particles->lremove(p1->prev);
	    }
	  // If we don't have a neighbor north of us
	  else
	    {
	      //Collide with wall
	      *local_momentum += wall_collide(p1->pcord, wall);
	      p1 = p1->next;
	    }
	}
      //Particle is within our area
      else
	{
	  for(int k=j;k<particles->lget_size();k++){

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
	  
	  //Collide with wall
	  *local_momentum += wall_collide(p1->pcord, wall);
	  p1 = p1->next;
	}
    }
    coll_reg.end();
    
  }

  //Sum momentum
  MPI_Reduce(local_momentum, global_momentum, 1, MPI_FLOAT, MPI_SUM, root, MPI_COMM_WORLD);
  //long long tot_sent;
  //MPI_Reduce(&sent_particles, &tot_sent, 1, MPI_LONG_LONG_INT, MPI_SUM, 0);
  
  VT_countval(1, &sent_particles_handle, &sent_particles);

  if(rank == root)
    {
      t2=clock();
      cout<< "pressure: " <<   calc_pressure(global_momentum) <<  endl;
      float sim_time = ((float)t2-(float)t1)/CLOCKS_PER_SEC;
      cout<< "Simulation finished after: " <<   sim_time <<  endl;
    }

  main_reg.end();
  MPI_Finalize();

  return 0;
}
