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

/* Frekvens "indices" */
static int frequencies[]  = {0,2,4,0,0,2,4,0,4,5,7,4,5,7,7,9,
                             7,5,4,0,7,9,7,5,4,0,0,-5,0,0,-5,0};

// Notlängder sparas som *2 för att ej behöva använda float
static int lengths[]      = {2,2,2,2,2,2,2,2,2,2,4,2,2,4,1,1,
                             1,1,2,2,1,1,1,1,2,2,2,2,4,2,2,4};

void receiver(App *self, int unused) {
  CANMsg msg;
  CAN_RECEIVE(&can0, &msg);
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "##Can msg recieved:\nmsgID: %d, Msg: ", msg.msgId);
  SCI_WRITE(&sci0, buffer);
  SCI_WRITE(&sci0, msg.buff);
  SCI_WRITE(&sci0, "\n");

  switch (msg.msgId) {
    case CAN_PLAY:  ASYNC(&mp, togglePlay,  atoi(msg.buff)); break;
    case CAN_MUTE:  ASYNC(&mp, toggleMute,  atoi(msg.buff)); break;
    case CAN_VOL:   ASYNC(&mp, setVolume,   atoi(msg.buff)); break;
    case CAN_TEMPO: ASYNC(&mp, setTempo,    atoi(msg.buff)); break;
    case CAN_KEY:   ASYNC(&mp, setKey,      atoi(msg.buff)); break;
    default: break;
  }
}

void sendCan(int m_id, int n_id, int len, char *buffer) {
  if (len > 7) return;
  CANMsg msg;

  msg.msgId = m_id;
  msg.nodeId = n_id;
  msg.length = len;

  for (int i = 0; i < len; i++) {
    msg.buff[i] = buffer[i];
  }
  msg.buff[len] = 0;

  CAN_SEND(&can0, &msg);
}

void reader(App *self, int c) {
  SCI_WRITE(&sci0, "Rcv: \'");
  SCI_WRITECHAR(&sci0, c);
  SCI_WRITE(&sci0, "\'\n");

  /* Hantera input */
  switch (c) {
    /* Direkta handlingar */
    case PLAYKEY:     ASYNC(&mp, togglePlay, !mp.play);     break;
    case MUTEKEY:     ASYNC(&mp, toggleMute, !tg.mute);     break;
    case VOLUPKEY:    ASYNC(&tg, setVolume, tg.volume + 1); break;
    case VOLDOWNKEY:  ASYNC(&tg, setVolume, tg.volume - 1); break;

    default: /* Integer input */

    if ( c != CANCELKEY && c != TEMPOKEY && c != KEYKEY && self->cnt < 11) { /* FORTSÄTT SKRIVA */
        self->buffer[self->cnt] = c;    // spara char till buffer[cnt]
        self->cnt++;                    // öka cnt
      }

      else { /* SLUTA SKRIVA */

        self->buffer[self->cnt] = '\0';                 // null-terminator
        int num = atoi(self->buffer);                   // ASCII to Integer(buffer)

        self->cnt = 0;                                  // Nollställ cnt
        memset(self->buffer, 0, sizeof(self->buffer));  // Rensa buffer

        switch (c) { /* Vad ska skickas */
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

  // Sparar msg för att kunna avbryta vid silence()
  self->pending = SEND(USEC(self->period), USEC(10), self, toneGenerator, 0);
}

int getPeriods(int index) {
  static int periods[] = {2025, 1911, 1803, 1702, 1607, 1516, 1431, 1351,
                          1275, 1203, 1136, 1072, 1012, 955, 901, 851,
                          803, 758, 715, 675, 637, 601, 568, 536, 506};

  // Indexera med +10 för att räkna med key som parsas i index
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
  // Om toneGenerator inte finns i kön
  if (!self->pending) self->pending = SEND(USEC(self->period), USEC(10), self, toneGenerator, 0);
}

void silence(ToneGenerator *self, int arg) {
  self->silence = 1;
  if (self->pending) { // Om call redan är i kö, ta bort, avbryt
    ABORT(self->pending);
    self->pending = NULL;
  }
}

void playNote(MusicPlayer *self, int arg) {
  // räknar massa skit
  int i           = self->index;
  Time beat       = MSEC(60000 / self->tempo);
  Time note_time  = beat * lengths[i] / 2;
  Time gap_time   = MSEC(50);
  Time play_time  = note_time - gap_time;

  // beat = 60s / bpm
  // note_time = beat * notlängd
  // gap_time = 50ms
  // play_time = note_time - gap_time
  // next_start = 10us (dl för setTone) + note_time
  // next_stop = beat * (nästa notlängd) - gap_time

  // Räkna tider för nästa not
  Time next_start = USEC(10) + note_time;
  Time next_stop = beat * (lengths[(i + 1) % 32] / 2) - gap_time;

  // Skapa kö
  SEND(0, USEC(10), &tg, setTone, getPeriods(frequencies[i] + self->key));
  SEND(play_time, gap_time, &tg, silence, 0);
  self->index = (self->index + 1) % 32;
  if (self->play) SEND(next_start, next_stop, self, playNote, 0);
}

void togglePlay(MusicPlayer *self, int arg) {
  if (arg) {
    self->play = 1;
    SCI_WRITE(&sci0, "Playback started\n");
    ASYNC(self, playNote, 0);
  } else {
    self->play = 0;
    SCI_WRITE(&sci0, "Playback stopped\n");
  }
}

void toggleMute(MusicPlayer *self, int arg) {
  if (arg) {
    tg.mute = 1;
    SCI_WRITE(&sci0, "Muted\n");
  } else {
    tg.mute = 0;
    SCI_WRITE(&sci0, "Unmuted\n");
  }
}

void startApp(App *self, int arg) {
  CAN_INIT(&can0);
  SCI_INIT(&sci0);
  SCI_WRITE(&sci0, "Cool musikspelare\n");
}

int main() {
  INSTALL(&sci0, sci_interrupt, SCI_IRQ0);
  INSTALL(&can0, can_interrupt, CAN_IRQ0);
  TINYTIMBER(&app, startApp, 0);
  return 0;
}
