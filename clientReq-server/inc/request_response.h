#ifndef _REQUEST_RESPONSE_HH
#define _REQUEST_RESPONSE_HH

//definizione strutture
struct Request{
  char id [100];
  char servizio [10];
};

struct Response{
  int key;
};

#endif