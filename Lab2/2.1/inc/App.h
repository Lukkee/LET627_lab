#ifndef _APP_H
#define _APP_H

#include "TinyTimber.h"

typedef struct {
  Object super;
  int example;
} App;

typedef struct {
  Object super;
  int key;
  int tempo;
} MusicPlayer;

typedef struct {
  Object super;
  int frequency;
  int volume;
  int muted;
} ToneGenerator;

#define initApp()                                                              \
  { initObject(), 1234 }

  #define initMusicPlayer()                                                              \
  { initObject(), 0, 120 }

#define initToneGenerator()                                                              \
  { initObject(), 1234, 3, 0 }

void reader(App *, int);
void receiver(App *, int);
void startApp(App *, int);

#endif
