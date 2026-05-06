#include "App.h"
#include "TinyTimber.h"
#include "canTinyTimber.h"
#include "sciTinyTimber.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    SCI_WRITE(&sci0, "Rcv: '");
    SCI_WRITECHAR(&sci0, c);
    SCI_WRITE(&sci0, "'\n");

    /* ---------- INPUT MODE ---------- */
    if (self->mode != INPUT_DEFAULT) {

        if (c != 'e' && self->cnt < 11) {
            self->buffer[self->cnt++] = c;
            return;
        }

        /* Finish input */
        self->buffer[self->cnt] = '\0';
        int value = atoi(self->buffer);

        if (self->mode == INPUT_KEY) {
            ASYNC(&mp, setKeyVal, value);
        }
        else if (self->mode == INPUT_TEMPO) {
            ASYNC(&mp, setTempoVal, value);
        }

        self->cnt = 0;
        self->mode = INPUT_DEFAULT;
        memset(self->buffer, 0, sizeof(self->buffer));

        return;
    }

    /* ---------- DEFAULT MODE ---------- */
    switch (c) {
        case MUTE:        ASYNC(&tg, toggleMute, 0); break;
        case VOL_UP:      ASYNC(&tg, increaseVolume, 0); break;
        case VOL_DOWN:    ASYNC(&tg, decreaseVolume, 0); break;
        case PAUSE:       ASYNC(&mp, pausePlayer, 0); break;
        case KEY_UP:      ASYNC(&mp, increaseKey, 0); break;
        case KEY_DOWN:    ASYNC(&mp, decreaseKey, 0); break;
        case TEMPO_UP:    ASYNC(&mp, increaseTempo, 0); break;
        case TEMPO_DOWN:  ASYNC(&mp, decreaseTempo, 0); break;
        case SETKEY:      ASYNC(&mp, setKey, 0); break;
        case SETTEMPO:    ASYNC(&mp, setTempo, 0); break;
        default: break;
    }
}


void setKey(MusicPlayer *self, int arg) {
    app.mode = INPUT_KEY;
    app.cnt = 0;
    memset(app.buffer, 0, sizeof(app.buffer));
    SCI_WRITE(&sci0, "Enter key, end with 'e'\n");
}

void setTempo(MusicPlayer *self, int arg) {
    app.mode = INPUT_TEMPO;
    app.cnt = 0;
    memset(app.buffer, 0, sizeof(app.buffer));
    SCI_WRITE(&sci0, "Enter tempo, end with 'e'\n");
}


void toggleMute(ToneGenerator *self, int arg) {
  self->usr_mute = !self->usr_mute;
}

void increaseVolume(ToneGenerator *self, int arg) {
  if (self->volume < VOL_MAX) self->volume++;

  char buffer[32];
  snprintf(buffer, sizeof(buffer), "Vol: %d\n", self->volume);
  SCI_WRITE(&sci0, buffer);
}

void decreaseVolume(ToneGenerator *self, int arg) {
  if (self->volume > VOL_MIN) self->volume--;

  char buffer[32];
  snprintf(buffer, sizeof(buffer), "Vol: %d\n", self->volume);
  SCI_WRITE(&sci0, buffer);
}

void pausePlayer(MusicPlayer *self, int arg) {
  self->pause = !self->pause;
  if (!self->pause) ASYNC(&mp, playNote, 0);
}

void increaseKey(MusicPlayer *self, int arg) {
  if (self->key < KEY_MAX) self->key++;

  char buffer[32];
  snprintf(buffer, sizeof(buffer), "Key: %d\n", self->key);
  SCI_WRITE(&sci0, buffer);
}

void decreaseKey(MusicPlayer *self, int arg) {
  if (self->key > KEY_MIN) self->key--;

  char buffer[32];
  snprintf(buffer, sizeof(buffer), "Key: %d\n", self->key);
  SCI_WRITE(&sci0, buffer);
}

void setKeyVal(MusicPlayer *self, int arg) {
  if (arg >= KEY_MIN && arg <= KEY_MAX) self->key = arg;

  char buffer[32];
  snprintf(buffer, sizeof(buffer), "Key: %d\n", self->key);
  SCI_WRITE(&sci0, buffer);
}

void increaseTempo(MusicPlayer *self, int arg) {
  if (self->tempo < TEMPO_MAX) self->tempo++;

  char buffer[32];
  snprintf(buffer, sizeof(buffer), "Tempo: %d\n", self->tempo);
  SCI_WRITE(&sci0, buffer);
}

void decreaseTempo(MusicPlayer *self, int arg) {
  if (self->tempo > TEMPO_MIN) self->tempo--;

  char buffer[32];
  snprintf(buffer, sizeof(buffer), "Tempo: %d\n", self->tempo);
  SCI_WRITE(&sci0, buffer);
}

void setTempoVal(MusicPlayer *self, int arg) {
  if (arg >= TEMPO_MIN && arg <= TEMPO_MAX) self->tempo = arg;

  char buffer[32];
  snprintf(buffer, sizeof(buffer), "Tempo: %d\n", self->tempo);
  SCI_WRITE(&sci0, buffer);
}

int getPeriods(int index) {
  static int periods[] = {2025, 1911, 1803, 1702, 1607, 1516, 1431, 1351,
                          1275, 1203, 1136, 1072, 1012, 955, 901, 851,
                          803, 758, 715, 675, 637, 601, 568, 536, 506};

  return periods[index + 10];
}

void silence(ToneGenerator *self, int arg) {
  self->muted = 1;
}

void toneGenerator(ToneGenerator *self, int arg) {
    static int toggle = 0;
    if (!self->muted && !self->usr_mute) {
        *DAC_DATA = toggle ? self->volume : 0;
        toggle = !toggle;
    } else {
        *DAC_DATA = 0;
    }
    SEND(USEC(self->period_us), 0, self, toneGenerator, 0);
}

void setNote(ToneGenerator *self, int arg) {
    self->period_us = arg;
    self->muted = 0;
}

void playNote(MusicPlayer *self, int arg) {
    int i = self->index;

    // beat = 60000ms / tempo
    Time beat      = MSEC(60000 / self->tempo);
    Time note_time = beat * lengths[i] / 2;  // /2 då de sparas som *2
    Time play_time = note_time - (note_time / 16);

    // Sätt frekvens på toneGenerator
    int freq_index = frequencies[i] + self->key;
    SYNC(&tg, setNote, getPeriods(freq_index));

    // Schemalägg mellanrum innan nästa not
    SEND(play_time, 0, &tg, silence, 0);

    // Schemalägg nästa not
    self->index = (self->index + 1) % 32;
    if (!self->pause) SEND(note_time, 0, self, playNote, 0);
}

void startApp(App *self, int arg) {
  CANMsg msg;

  CAN_INIT(&can0);
  SCI_INIT(&sci0);
  SCI_WRITE(&sci0, "Hello, hello...\n");

  msg.msgId = 1;
  msg.nodeId = 1;
  msg.length = 6;
  msg.buff[0] = 'H';
  msg.buff[1] = 'e';
  msg.buff[2] = 'l';
  msg.buff[3] = 'l';
  msg.buff[4] = 'o';
  msg.buff[5] = 0;
  CAN_SEND(&can0, &msg);

  ASYNC(&tg, toneGenerator, 0);
  ASYNC(&mp, playNote, 0);
}

int main() {
  INSTALL(&sci0, sci_interrupt, SCI_IRQ0);
  INSTALL(&can0, can_interrupt, CAN_IRQ0);
  TINYTIMBER(&app, startApp, 0);
  return 0;
}
