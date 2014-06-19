#include "ll.h"
#include <iostream>

using namespace std;

ll::ll()
{
  linit();
}

ll::~ll()                 
{
  lclear();
}


void ll::linit()
{
  m_size = 0;
  m_id = 0;
  m_last = NULL;
  m_first = NULL;
  return;
}

void ll::ladd_last(pcord_t* cord)
{

  //Allocate memory
  p* particle = (p* ) malloc(sizeof(p));


  //Set all variables
  particle->id = m_id;
  particle->pcord = cord;
  particle->prev = m_last;
  particle->next = NULL;

  //If the list is empty
  if(m_size == 0)
    m_first = particle;
  else
    m_last->next = particle;

  //Make the new particle the last
  m_last = particle;
  m_id++;
  m_size++;

  return;
}

void ll::lremove(p* particle)
{
  //If its only one item left
  if(particle == m_last && particle == m_first)
    {
      m_first = NULL;
      m_last = NULL;
    }
  //If its the last item
  else if(particle == m_last)
    {
      m_last = particle->prev;
    }  
  //If its the first item
  else if(particle == m_first)
    {
      m_first = particle->next;
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
  m_size--;
  return;
}


//Clear the entire list
void ll::lclear()
{
 
  while(m_size > 0)
    {
      lremove(m_first);
    }

}

// Print ids
void ll::lprint_id()
{
  p* temp = lget_first();
  int i = 0;
  for(;i<m_size;i++)
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
  return;
}

int ll::lget_size(void)
{
  return m_size;
}

p* ll::lget_last(void)
{
  return m_last;
}

p* ll::lget_first(void)
{
  return m_first;
}
