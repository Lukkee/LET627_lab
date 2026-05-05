#include "App.h"
#include "TinyTimber.h"
#include "canTinyTimber.h"
#include "sciTinyTimber.h"
#include <stdio.h>

extern App app;
extern MusicPlayer mp;
extern ToneGenerator tg;
extern Can can0;
extern Serial sci0;

static int frequencies[]  = {0,2,4,0,0,2,4,0,4,5,7,4,5,7,7,9,
                             7,5,4,0,7,9,7,5,4,0,0,-5,0,0,-5,0};

static int lengths[]      = {2,2,2,2,2,2,2,2,2,2,4,2,2,4,1,1,
                             1,1,2,2,1,1,1,1,2,2,2,2,4,2,2,4};

void receiver(App *self, int unused) {
  CANMsg msg;
  CAN_RECEIVE(&can0, &msg);
  SCI_WRITE(&sci0, "Can msg received: ");
  SCI_WRITE(&sci0, msg.buff);
}

void reader(App *self, int c) {

    SCI_WRITE(&sci0, "Rcv: \'");
    SCI_WRITECHAR(&sci0, c);
    SCI_WRITE(&sci0, "\'\n");

  // Input hantering
    switch (c) {
        case MUTE:        ASYNC(&tg, toggleMute,      0);          break; // MUTE
        case VOL_UP:      ASYNC(&tg, increaseVolume,  0);          break; // ÖKA  VOLYM
        case VOL_DOWN:    ASYNC(&tg, decreaseVolume,  0);          break; // SÄNK VOLYM
        case KEY_UP:      ASYNC(&mp, increaseKey,     0);          break; // ÖKA  KEY
        case KEY_DOWN:    ASYNC(&mp, decreaseKey,     0);          break; // SÄNK KEY
        case TEMPO_UP:    ASYNC(&mp, increaseTempo,   0);          break; // ÖKA  TEMPO
        case TEMPO_DOWN:  ASYNC(&mp, decreaseTempo,   0);          break; // SÄNK TEMPO

        default: break;
    }
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

int getFrequencies(int index) {
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
    Time note_time = beat * lengths[i] / 2;  // /2 because stored as *2
    Time play_time = note_time - (note_time / 16);

    // Set frequency on ToneGenerator
    int freq_index = frequencies[i] + self->key;
    SYNC(&tg, setNote, getFrequencies(freq_index));

    // Schedule silence after play_time
    SEND(play_time, 0, &tg, silence, 0);

    // Schedule next note
    self->index = (self->index + 1) % 32;
    SEND(note_time, 0, self, playNote, 0);
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
