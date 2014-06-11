
#ifndef _ll_h
#define _ll_h

#include "coordinate.h"
#include "definitions.h"


//Linked list struct
struct p {
  int id; //Debugg only
  pcord_t* pcord;
  p* next;
  p* prev;
};

//Global variables for the list
extern int size;
extern int id;
extern p* last;
extern p* first;

//Public functions
void linit(void);

void ladd_last(pcord_t* cord);

void lremove(p* particle);

void lclear();

int lget_size(void);

p* lget_last(void);

p* lget_first(void);

void lprint_id(void);

#endif
