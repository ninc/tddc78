#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "ll.h"


using namespace std;

int size;
int id;
p* last;
p* first;

void linit()
{
  size = 0;
  id = 0;
  last = NULL;
  first = NULL;
}


void ladd_last(pcord_t* cord)
{

  //Allocate memory
  p* particle = (p* ) malloc(sizeof(p));


  //Set all variables
  particle->id = id;
  particle->pcord = cord;
  particle->prev = last;
  particle->next = NULL;

  //If the list is empty
  if(size == 0)
    first = particle;
  else
    last->next = particle;

  //Make the new particle the last
  last = particle;
  id++;
  size++;
  return;
}

void lremove(p* particle)
{
  //If its only one item left
  if(particle == last && particle == first)
    {
      first = NULL;
      last = NULL;
    }
  //If its the last item
  else if(particle == last)
    {
      last = particle->prev;
    }  
  //If its the first item
  else if(particle == first)
    {
      first = particle->next;
    }
  //If its an item between other items
  else
    {
      particle->prev->next = particle->next;
      particle->next->prev = particle->prev;
    }

  //Remove self
  free(particle);

  particle = NULL;
  size--;

  return;
}


//Clear the entire list
void lclear()
{
 
  while(size > 0)
    {
      lremove(first);
    }

}

// Print ids
void lprint_id()
{
  p* temp = lget_first();
  int i = 0;
  for(;i<size;i++)
    {
      if(temp == NULL)
	{
	  break;
	}
      else
	{
	  cout << "i: " << i << "ID: " << temp->id << " ";

	  temp = temp->next;
	}
    }

  cout << endl;
}

int lget_size(void)
{
  return size;
}

p* lget_last(void)
{
  return last;
}

p* lget_first(void)
{
  return first;
}
