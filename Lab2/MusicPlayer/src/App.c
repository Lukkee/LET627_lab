#include "App.h"
#include "TinyTimber.h"
#include "canTinyTimber.h"
#include "sciTinyTimber.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern App app;
extern MusicPlayer mp;
extern ToneGenerator tg;
extern Can can0;
extern Serial sci0;

static int frequencies[]  = {0,2,4,0,0,2,4,0,4,5,7,4,5,7,7,9,
                             7,5,4,0,7,9,7,5,4,0,0,-5,0,0,-5,0};

// Sparas som *2 för att ej behöva använda float
static int lengths[]      = {2,2,2,2,2,2,2,2,2,2,4,2,2,4,1,1,
                             1,1,2,2,1,1,1,1,2,2,2,2,4,2,2,4};

void receiver(App *self, int unused) {
  CANMsg msg;
  CAN_RECEIVE(&can0, &msg);
  SCI_WRITE(&sci0, "Can msg received: ");
  SCI_WRITE(&sci0, msg.buff);
}

void reader(App *self, int c) {

  switch (c) {
    case MUTEKEY:     tg.mute = !tg.mute; break;
    case VOLUPKEY:    ASYNC(&tg, setVolume, tg.volume + 1); break;
    case VOLDOWNKEY:  ASYNC(&tg, setVolume, tg.volume - 1); break;

    default: /* Integers */
      if ( c != CANCELKEY && c != TEMPOKEY && c != KEYKEY && self->cnt < 11) { /* FORTSÄTT SKRIVA */
        self->buffer[self->cnt] = c;    // spara char till buffer[cnt]
        self->cnt++;                    // öka cnt
      }
      else { /* SLUTA SKRIVA */
        self->buffer[self->cnt] = '\0';                 // null-terminator
        int num = atoi(self->buffer);                   // ASCII to Integer(buffer)
        self->cnt = 0;                                  // Nollställ cnt
        memset(self->buffer, 0, sizeof(self->buffer));  // Rensa buffer

        switch (c) {
          case TEMPOKEY:  ASYNC(&mp, setTempo, num); break;
          case KEYKEY:    ASYNC(&mp, setKey, num); break;
          default:  break;
        }
      } break;
  }
}

void toneGenerator(ToneGenerator *self, int arg) {
  if (!self->silence && !self->mute) {
    *DAC_DATA = self->toggle ? self->volume : 0;
    self->toggle = !self->toggle;
  } else {
    *DAC_DATA = 0;
  }

  self->pending = SEND(USEC(self->period), USEC(10), self, toneGenerator, 0);
}

int getPeriods(int index) {
  static int periods[] = {2025, 1911, 1803, 1702, 1607, 1516, 1431, 1351,
                          1275, 1203, 1136, 1072, 1012, 955, 901, 851,
                          803, 758, 715, 675, 637, 601, 568, 536, 506};

  return periods[index + 10];
}

void setTempo(MusicPlayer *self, int arg) {
  if (arg <= MAX_TEMPO && arg >= MIN_TEMPO) self->tempo = arg;

  char buff[16];
  snprintf(buff, sizeof(buff), "Tempo: %d\n", self->tempo);
  SCI_WRITE(&sci0, buff);
}

void setKey(MusicPlayer *self, int arg) {
  if (arg <= MAX_KEY && arg >= MIN_KEY) self->key = arg;

  char buff[16];
  snprintf(buff, sizeof(buff), "Key: %d\n", self->key);
  SCI_WRITE(&sci0, buff);
}

void setVolume(ToneGenerator *self, int arg) {
  if (arg <= MAX_VOL && arg >= MIN_VOL) self->volume = arg;

  char buff[16];
  snprintf(buff, sizeof(buff), "Vol: %d\n", self->volume);
  SCI_WRITE(&sci0, buff);
}

void setTone(ToneGenerator *self, int arg) {
  self->period = arg;
  self->silence = 0;
  if (!self->pending) self->pending = SEND(USEC(self->period), USEC(10), self, toneGenerator, 0);
}

void silence(ToneGenerator *self, int arg) {
  self->silence = 1;
  if (self->pending) {
  ABORT(self->pending);
  self->pending = NULL;
  }
}

void playNote(MusicPlayer *self, int arg) {
  // räknar massa skit
  int i = self->index;
  Time beat = MSEC(60000 / self->tempo);
  Time note_time = beat * lengths[i] / 2;
  Time gap_time = MSEC(50);
  Time play_time = note_time - gap_time;

  // beat = 60s / bpm
  // note_time = beat * notlängd
  // gap_time = 50ms
  // play_time = note_time - gap_time
  // next_start = 10us (dl för setTone) + note_time
  // next_stop = beat * (nästa notlängd) - gap_time

  Time next_start = USEC(10) + note_time;
  Time next_stop = beat * (lengths[(i + 1) % 32] / 2) - gap_time;

  SEND(0, USEC(10), &tg, setTone, getPeriods(frequencies[i] + self->key));
  SEND(play_time, gap_time, &tg, silence, 0);
  self->index = (self->index + 1) % 32;
  SEND(next_start, next_stop, &mp, playNote, 0);
}


void startApp(App *self, int arg) {
  CAN_INIT(&can0);
  SCI_INIT(&sci0);
  SCI_WRITE(&sci0, "Cool j\x84vla musikspelare\n");

  ASYNC(&mp, playNote, 0);
}

int main() {
  INSTALL(&sci0, sci_interrupt, SCI_IRQ0);
  INSTALL(&can0, can_interrupt, CAN_IRQ0);
  TINYTIMBER(&app, startApp, 0);
  return 0;
}
