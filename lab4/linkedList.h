
#include "coordinate.h"
#include "definitions.h"


//Linked list struct
struct p {
  int id; //Debugg only
  pcord_t pcord;
  p* next;
  p* prev;
};

//Global variables for the list
int size;
int id;
p* last;
p* first;

//Public functions
void init(void);

void add_last(p* particle);

void remove(p* particle);

void clear();

int get_size(void);

p* get_last(void);

p* get_first(void);

