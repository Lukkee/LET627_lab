#ifndef _APP_H
#define _APP_H

#include "TinyTimber.h"

typedef struct {
  Object super;
  Time now;
  Time last;
  int pressed;
  int mode;
  Msg pending;
} App;

#define initApp()                                                              \
  { initObject(), 1234 }

void reader(App *, int);
void receiver(App *, int);
void startApp(App *, int);
void SioCallback(App *, int);
void checkHold(App *, int);

#endif
