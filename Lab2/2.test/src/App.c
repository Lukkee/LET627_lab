#include "App.h"
#include "TinyTimber.h"
#include "canTinyTimber.h"
#include "sciTinyTimber.h"
#include <stdlib.h>

extern App app;
extern MusicPlayer mp;
extern ToneGenerator tg;
extern Can can0;
extern Serial sci0;

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
}

void toneGenerator(ToneGenerator *self, int arg) {
  if (!self->silence) {
    *DAC_DATA = self->toggle ? self->volume : 0;
    self->toggle = !self->toggle;
  } else {
    *DAC_DATA = 0;
  }
  SEND(USEC(self->period), 0, self, toneGenerator, 0);
}

void setTone(ToneGenerator *self, int arg) {
  self->period = arg;
}

void silence(ToneGenerator *self, int arg) {
  self->silence = 1;
}

void playNote(MusicPlayer *self, int arg) {
  // test var
  int length = 1;

  // räknar massa skit
  Time beat = MSEC(60000 / self-tempo);
  Time note_time = beat * length;
  Time gap_time = MSEC(50);
  Time play_time = note_time - gap_time;

  //SEND  (bl, b,   silence)  // gap
  //SEND  (bl, dl,  setTone)  // ändra ton
  //SEND  (b, dl,   playNote) // nästa not
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
}

int main() {
  INSTALL(&sci0, sci_interrupt, SCI_IRQ0);
  INSTALL(&can0, can_interrupt, CAN_IRQ0);
  TINYTIMBER(&app, startApp, 0);
  return 0;
}
