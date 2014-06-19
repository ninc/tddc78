
#ifndef _LL_H
#define _LL_H

#include "coordinate.h"
#include "definitions.h"



//Linked list struct
struct p {
  int id;
  pcord_t* pcord;
  p* next;
  p* prev;
};

class ll
{
 public:
  ll();
  ~ll(); 
  void ladd_last(pcord_t* cord);
  void lremove(p* particle);
  void lclear();
  int lget_size(void);
  p* lget_last(void);
  p* lget_first(void);
  void lprint_id(void);

 private:  
  void linit(void);
 
  int m_size;
  int m_id;
  p* m_last;
  p* m_first;
};

#endif
