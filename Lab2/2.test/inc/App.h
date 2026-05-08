#ifndef _APP_H
#define _APP_H

#include "TinyTimber.h"

#define DAC_DATA ((volatile uint8_t *)0x4000741C)

/* PARAMETRAR */
#define MAX_TEMPO 240
#define MIN_TEMPO 60
#define MAX_KEY 5
#define MIN_KEY -5
#define MAX_VOL 20
#define MIN_VOL 0

/* KONTROLLER */
#define TEMPOKEY    't'
#define KEYKEY      'k'
#define MUTEKEY     'm'
#define VOLUPKEY    '+'
#define VOLDOWNKEY  '-'
#define CANCELKEY   'c'
#define HELPKEY     'h'

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
  { initObject(), 0, 3, 1203, 1, 0 }

/* APP */
void reader(App *, int);
void receiver(App *, int);
void startApp(App *, int);

/* TONEGENERATOR */
void toneGenerator(ToneGenerator *, int);
void silence(ToneGenerator *, int);
void setTone(ToneGenerator *, int);
void setVolume(ToneGenerator *, int);


/* MUSICPLAYER */
void playNote(MusicPlayer *, int);
void setTempo(MusicPlayer *, int);
void setKey(MusicPlayer *, int);

#endif
