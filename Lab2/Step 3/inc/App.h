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
#define VOLUPKEY    'u'
#define VOLDOWNKEY  'd'
#define VOLKEY      'v'
#define CANCELKEY   'c'
#define PLAYKEY     'p'
#define MODEKEY     '.'

/* CAN MSGID */
#define CAN_PLAY    0
#define CAN_MUTE    1
#define CAN_VOL     2
#define CAN_TEMPO   3
#define CAN_KEY     4
#define CAN_INCVOL  5
#define CAN_DECVOL  6

typedef struct {
  Object super;
  int cnt;
  int mode;
  char buffer[7];
} App;

typedef struct {
  Object super;
  int key;
  int tempo;
  int index;
  int play;
} MusicPlayer;

typedef struct {
  Object super;
  int toggle;
  int volume;
  int period;
  int silence;
  int mute;
  Msg pending;
} ToneGenerator;

#define initApp()                                                              \
  { initObject(), 0, 1 }

#define initMusicPlayer()                                                              \
  { initObject(), 0, 120, 0 , 0}

#define initToneGenerator()                                                              \
  { initObject(), 0, 3, 1203, 1, 0 }

/* APP */
void reader(App *, int);
void musicianReader(App *, int);
void conductorReader(App *, int);
void receiver(App *, int);
void startApp(App *, int);

/* TONEGENERATOR */
void toneGenerator(ToneGenerator *, int);
void silence(ToneGenerator *, int);
void setTone(ToneGenerator *, int);
void setVolume(ToneGenerator *, int);
void incVolume(ToneGenerator *, int);
void decVolume(ToneGenerator *, int);


/* MUSICPLAYER */
void playNote(MusicPlayer *, int);
void setTempo(MusicPlayer *, int);
void setKey(MusicPlayer *, int);
void togglePlay(MusicPlayer *, int);
void toggleMute(MusicPlayer *, int);

#endif
