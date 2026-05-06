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
#define PAUSE       'p'
#define SETKEY      'k'
#define SETTEMPO    't'

#define VOL_MAX     20
#define VOL_MIN      0
#define KEY_MAX      5
#define KEY_MIN     -5
#define TEMPO_MAX  240
#define TEMPO_MIN   60

#define DAC_DATA ((volatile uint8_t *)0x4000741C)

typedef enum {
  INPUT_DEFAULT,
  INPUT_KEY,
  INPUT_TEMPO,
} InputMode;

typedef struct {
  Object super;
  InputMode mode;
  int cnt;
  char buffer[12];
} App;

typedef struct {
  Object super;
  int key;
  int tempo;
  int index;
  int pause;
} MusicPlayer;

typedef struct {
  Object super;
  int volume;
  int muted;
  int usr_mute;
  int period_us;
} ToneGenerator;

#define initApp()                                                              \
  { initObject(), INPUT_DEFAULT, 0 }

  #define initMusicPlayer()                                                              \
  { initObject(), 0, 120, 0, 0 }

#define initToneGenerator()                                                              \
  { initObject(), 3, 0, 0, 500}

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
void pausePlayer(MusicPlayer *, int);
void setKey(MusicPlayer *, int);
void setTempo(MusicPlayer *, int);
void setTempoVal(MusicPlayer *, int);
void setKeyVal(MusicPlayer *, int);

void toneGenerator(ToneGenerator *, int);
void silence(ToneGenerator *, int);
void playNote(MusicPlayer *, int);
void setNote(ToneGenerator *, int);

#endif
