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
        case MUTE:        ASYNC(&mp, toggleMute,    0);          break; // MUTE
        case VOL_UP:      ASYNC(&mp, increaseVolume 0);          break; // ÖKA  VOLYM
        case VOL_DOWN:    ASYNC(&mp, decreaseVolume 0);          break; // SÄNK VOLYM
        case KEY_UP:      ASYNC(&mp, increaseKey,   0);          break; // ÖKA  KEY
        case KEY_DOWN:    ASYNC(&mp, decreaseKey,   0);          break; // SÄNK KEY
        case TEMPO_UP:    ASYNC(&mp, increaseTempo, 0);          break; // ÖKA  TEMPO
        case TEMPO_DOWN:  ASYNC(&mp, decreaseTempo, 0);          break; // SÄNK TEMPO

        default: break;
    }
}

void toggleMute(MusicPlayer *self, int arg) {}

void increaseVolume(MusicPlayer *self, int arg) {}

void decreaseVolume(MusicPlayer *self, int arg) {}

void increaseKey(MusicPlayer *self, int arg) {}

void decreaseKey(MusicPlayer *self, int arg) {}

void increaseTempo(MusicPlayer *self, int arg) {}

void decreaseTempo(MusicPlayer *self, int arg) {}

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
