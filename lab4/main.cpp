// Mädz super duper particle simulation (MSDPS)
//icpc -o main main.cpp physics.c
//./main

#include <iostream>
#include "definitions.h"
#include "coordinate.h"
#include "physics.h"
#include <math.h>
#include <time.h>

using namespace std;

#define max_vel 50
#define max_l 10000
#define n 16000
#define t 50

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
int simulate(particle_t particles[n], cord_t wall)
{
  //begin simulation
  float collision = 0;
  float global_momentum = 0;
  for(int i=0;i<t;i++){
    
    //Broadcast particles

    //main loop
    for(int j=0;j<n;j++){
      for(int k=j+1;k<n;k++){
	 //Check for collisions
	collision = collide(&particles[j].pcord, &particles[k].pcord);
	//if collision
	if(collision != -1){
	  interact(&particles[j].pcord, &particles[k].pcord, collision);
	  break;
	}
      }
      //if no collision
      if(collision == -1) {
	//Move particles that have not collided
        feuler(&particles[j].pcord, i);
      }
      global_momentum += wall_collide(&particles[j].pcord, wall);
    }
    
    // Communicate if needed bastard!
    
  }
  return global_momentum;
}



// Calculate the pressure and verify that the calculations are correct
float calc_pressure(float momentum)
{

  return momentum /= (t*4*max_l);
}

int main (int argc, char ** argv) {
  particle_t particles[n];
  float momentum;
  cout<< "begin init particles" << endl;
  // Initialize simulation
  
  cord_t wall = init(particles);
  cout<< "finished init particles" << endl;
  clock_t t1,t2;
  t1 = clock();
  momentum = simulate(particles, wall);
  //calculate the total pressure of the bag-in-box
  t2=clock();
  cout<< "pressure: " <<   calc_pressure(momentum) <<  endl;
  float sim_time = ((float)t2-(float)t1)/CLOCKS_PER_SEC;
  cout<< "Simulation finished after: " <<   sim_time <<  endl;
  //clean_particles(particles);
  return 0;
}
