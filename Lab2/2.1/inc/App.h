#ifndef _APP_H
#define _APP_H

#include "TinyTimber.h"

#define MUTE        'm'
#define VOL_UP      '+'
#define VOL_DOWN    '-'
#define KEY_UP      'u'
#define KEY_DOWN    'd'
#define TEMPO_UP    'e'
#define TEMPO_DOWN  'q'

#define VOL_MAX     20
#define VOL_MIN      0
#define KEY_MAX      5
#define KEY_MIN     -5
#define TEMPO_MAX  340
#define TEMPO_MIN   30

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

void toggleMute(ToneGenerator *, int);
void increaseVolume(ToneGenerator *, int);
void decreaseVolume(ToneGenerator *, int);
void increaseKey(MusicPlayer *, int);
void decreaseKey(MusicPlayer *, int);
void increaseTempo(MusicPlayer *, int);
void decreaseTempo(MusicPlayer *, int);

void toneGenerator(ToneGenerator *, int);

#endif
