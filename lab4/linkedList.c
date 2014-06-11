#include <stdlib.h>
#include <stdio.h>
#include "linkedlist.h"





void init()
{
  size = 0;
  id = 0;
  p* last = NULL;
  p* first = NULL;
}


void add_last(pcort_t cord)
{
  id++;
  size++;
  //Allocate memory
  p* particle = malloc(sizeof(p));

  //Set all variables
  particle->id = id;
  particle->pcord = cord;
  particle->prev = last;
  particle->next = NULL;

  //Make the new particle the last
  last->next = particle;
  last = particle;

  return;
}

void remove(p* particle)
{
  size--;
  if(particle == last)
    {
      last = particle->prev;
    }
  
  if(particle == first)
    {
      first = particle->next;
    }

  //Set neighbouring pointers
  particle->prev->next = particle->next;
  particle->next->prev = particle->prev;

  //Remove self
  free(particle);

  return;
}


//Clear the entire list
void clear()
{
 
  while(size > 0)
    {
      remove(first);
    }

}


int get_size(void)
{
  return size;
}

p* get_last(void)
{
  return last;
}

p* get_first(void)
{
  return first;
}
