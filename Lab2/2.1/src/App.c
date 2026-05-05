#include "App.h"
#include "TinyTimber.h"
#include "canTinyTimber.h"
#include "sciTinyTimber.h"

extern App app;
extern MusicPlayer mp;
extern ToneGenerator tg;
extern Can can0;
extern Serial sci0;

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
  self->muted = !self->muted;
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

void toneGenerator(ToneGenerator *self, int arg) {}

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
}

int main() {
  INSTALL(&sci0, sci_interrupt, SCI_IRQ0);
  INSTALL(&can0, can_interrupt, CAN_IRQ0);
  TINYTIMBER(&app, startApp, 0);
  return 0;
}
