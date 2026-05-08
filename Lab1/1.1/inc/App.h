#ifndef _APP_H
#define _APP_H

#include "TinyTimber.h"

#define DAC_DATA ((volatile uint8_t *)0x4000741C)

typedef struct {
  Object super;
  int history[3];
  int history_index;
  char buffer[12];
  int cnt;
  int volume;
  int muted;
} App;

#define initApp()                                                              \
  { initObject(), 0 } // Initierar värden till objektet (self)

// Bas
void reader(App *, int);
void receiver(App *, int);
void startApp(App *, int);

// Egna
void toneGenerator(App *, int);
void volumeUp(App *, int);
void volumeDown(App *, int);
void toggleMute(App *, int);

#endif
