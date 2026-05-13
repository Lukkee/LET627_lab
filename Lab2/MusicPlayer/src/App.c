#include "App.h"
#include "TinyTimber.h"
#include "canTinyTimber.h"
#include "sciTinyTimber.h"
#include "sioTinyTimber.h"
#include "stm32f4xx_tim.h"  // timer

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern App app;
extern MusicPlayer mp;
extern ToneGenerator tg;
extern Can can0;
extern Serial sci0;
extern SysIO sio0;
extern Button btn;

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
  snprintf(buffer, sizeof(buffer), "##Can msg recieved: msgID: %d, Msg: ", msg.msgId);
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
    if (c == MODEKEY) {
        self->conductormode = !self->conductormode;
        if (self->conductormode) {
          SCI_WRITE(&sci0, "Conductor mode\n");
        } else {
           SCI_WRITE(&sci0, "Musician mode\n");
        }
        return;
    }

    if (self->conductormode) {
        conductorReader(self, c);
    } else {
        musicianReader(self, c);
    }
}

void musicianReader(App *self, int c) {
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

void conductorReader(App *self, int c) {
  SCI_WRITE(&sci0, "Rcv: \'");
  SCI_WRITECHAR(&sci0, c);
  SCI_WRITE(&sci0, "\'\n");

  /* Hantera input */
  if ( c != CANCELKEY
    && c != TEMPOKEY
    && c != KEYKEY
    && c != VOLKEY
    && c != MUTEKEY
    && c != PLAYKEY
    && self->cnt < 11) { /* FORTSÄTT SKRIVA */
      self->canbuffer[self->cnt] = c;    // spara char till canbuffer[cnt]
      self->cnt++;                    // öka cnt
  }

  else { /* SLUTA SKRIVA */

      self->canbuffer[self->cnt] = '\0';                 // null-terminator

      switch (c) { /* Vad ska skickas */
        case TEMPOKEY:
          sendCan(CAN_TEMPO, 1, strlen(self->canbuffer), self->canbuffer);
        break;
        case KEYKEY:
          sendCan(CAN_KEY, 1, strlen(self->canbuffer), self->canbuffer);
        break;
        case 'v':
          sendCan(CAN_VOL, 1, strlen(self->canbuffer), self->canbuffer);
        break;
        case 'm':
          sendCan(CAN_MUTE, 1, strlen(self->canbuffer), self->canbuffer);
        break;
        case 'p':
          sendCan(CAN_PLAY, 1, strlen(self->canbuffer), self->canbuffer);
        break;

        default: break;
        }

      self->cnt = 0;                                        // Nollställ cnt
      memset(self->canbuffer, 0, sizeof(self->canbuffer));  // Rensa canbuffer
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

void checkHold(Button *self, int arg) {
  if (self->pressed) {                    // Om pressed fortfarande är igång
    self->mode = 1;                       // Sätt mode
    SCI_WRITE(&sci0, "Entered PRESS-TO-HOLD\n");
    ASYNC(&mp, setTempo, 120);            // Sätt default-tempo
  }
}

void SioCallback(Button *self, int arg) {
  Time since_last = T_SAMPLE(&self->timer);
  if (since_last < MSEC(100) && (self->count > 0 || self->pressed)) {
    return; // Filtrera undan contact bounces
  }

  char buffer[32];

  if (!self->pressed) {
    self->pressed = 1;
    self->pending = AFTER(SEC(2), self, checkHold, 0);
    SIO_TRIG(&sio0, 1);
  } else {
    int diff_ms = since_last / 100;
    self->pressed = 0;
    T_RESET(&self->timer);
    ABORT(self->pending);
    SIO_TRIG(&sio0, 0);

    if (self->mode) {
      self->mode = 0;
      snprintf(buffer, sizeof(buffer), "Held for: %ds\n", diff_ms / 1000);
      SCI_WRITE(&sci0, buffer);
    } else {                                      // Om checkHold ej gått igenom
      SCI_WRITE(&sci0, "MOMENTARY-PRESS\n");
      if (diff_ms < 3000) {                  // Om intervallet inte för högt
        self->history[0] = self->history[1];
        self->history[1] = self->history[2];    // Flytta bak rest
        self->history[2] = diff_ms;           // Nytt värde i [2]
        self->count++;
        snprintf(buffer, sizeof(buffer), "Interval: %dms\n", diff_ms);
        SCI_WRITE(&sci0, buffer);
      } else {
        memset(self->history, 0, sizeof(self->history));
        self->count = 0;
      }
      if (self->count >= 3) {
        int avg = (self->history[0] + self->history[1] + self->history[2]) / 3;
        int bpm = 60000 / avg;
        ASYNC(&mp, setTempo, bpm);
      }
    }
  }
}

void EXTI9_5_IRQHandler(void) {
  sio_interrupt(&sio0, 0);
}

void startApp(App *self, int arg) {
  CAN_INIT(&can0);
  SCI_INIT(&sci0);
  SCI_WRITE(&sci0, "Cool musikspelare\n");

  SIO_INIT(&sio0);
  SIO_TRIG(&sio0, 1);

  T_RESET(&btn.timer);
}

int main() {
  INSTALL(&sci0, sci_interrupt, SCI_IRQ0);
  INSTALL(&can0, can_interrupt, CAN_IRQ0);
  INSTALL(&sio0, sio_interrupt, SIO_IRQ0);
  TINYTIMBER(&app, startApp, 0);
  return 0;
}
