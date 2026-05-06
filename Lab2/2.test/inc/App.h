#ifndef _APP_H
#define _APP_H

#include "TinyTimber.h"

#define DAC_DATA ((volatile uint8_t *)0x4000741C)

typedef struct {
  Object super;
  int cnt;
  char buffer[12];
} App;

typedef struct {
  Object super;
  int key;
  int tempo;
  int index;
} MusicPlayer;

typedef struct {
  Object super;
  int toggle;
  int volume;
  int period;
  int silence;
  int mute;
} ToneGenerator;

#define initApp()                                                              \
  { initObject(), 0 }

#define initMusicPlayer()                                                              \
  { initObject(), 0, 120, 0 }

#define initToneGenerator()                                                              \
  { initObject(), 0, 3, 500, 0, 0 }

void reader(App *, int);
void receiver(App *, int);
void startApp(App *, int);

#endif
