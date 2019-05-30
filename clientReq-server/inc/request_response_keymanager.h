#ifndef _REQUEST_RESPONSE_KEYMANAGER_HH
#define _REQUEST_RESPONSE_KEYMANAGER_HH

//definizione strutture
struct Request{
  pid_t processID;
  char identificativo [50];
  char servizio [7];
};

struct Response{
  int key;
};

struct keyManager{
  char identificativo[50];
  int key;
  time_t timestamp;
};

#endif
