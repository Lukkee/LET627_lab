#ifndef _APP_H
#define _APP_H

#include "TinyTimber.h"

typedef struct {
  Object super;
  int history[3];
  int history_index;
  char buffer[12];
  int cnt;
  int indices[32];
<<<<<<< HEAD
  double periods[32];

=======
  double periods[25];
>>>>>>> bb35828722915874f4d72624aa3dab83ca704cbb
} App;

#define initApp()                                                              \
  { initObject(), 1234 }

void reader(App *, int);
void receiver(App *, int);
void startApp(App *, int);

#endif
