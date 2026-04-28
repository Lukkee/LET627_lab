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
  int period_us;
  int deadline;
} App;

typedef struct {
  Object super;
  int background_loop_range;
  int deadline;
} BackgroundTask;

// Initierar värden till objekt
#define initApp() \
  { initObject(), 0 }

#define initBackgroundTask() \
  { initObject(), 1000 }

// Bas
void reader(App *, int);
void receiver(App *, int);
void startApp(App *, int);

// Egna
void toneGenerator(App *, int);
void volumeUp(App *, int);
void volumeDown(App *, int);
void toggleMute(App *, int);

void setFrequency(App *self, int freq);
void toggleDeadline(App *self, int);
void toggleDeadlineBG(BackgroundTask *self, int);

void backgroundLoad(BackgroundTask *, int);
void increaseLoad(BackgroundTask *, int);
void decreaseLoad(BackgroundTask *, int);

#endif
