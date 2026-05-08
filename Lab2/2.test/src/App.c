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
  SEND(USEC(self->period), USEC(10), self, toneGenerator, 0);
}

int getPeriods(int index) {
  static int periods[] = {2025, 1911, 1803, 1702, 1607, 1516, 1431, 1351,
                          1275, 1203, 1136, 1072, 1012, 955, 901, 851,
                          803, 758, 715, 675, 637, 601, 568, 536, 506};

  return periods[index + 10];
}

void setTone(ToneGenerator *self, int arg) {
  self->period = arg;
  self->silence = 0;
}

void silence(ToneGenerator *self, int arg) {
  self->silence = 1;
}

void playNote(MusicPlayer *self, int arg) {
  // räknar massa skit
  int i = self->index;
  Time beat = MSEC(60000 / self->tempo);
  Time note_time = beat * lengths[i] / 2;
  Time gap_time = MSEC(50);
  Time play_time = note_time - gap_time;

  char buff[16];
  snprintf(buff, sizeof(buff), "%d : %d\n", frequencies[i], lengths[i]);
  SCI_WRITE(&sci0, buff);

  Time next_start = USEC(10) + note_time;
  Time next_stop = beat * (lengths[(i + 1) % 32] / 2) - gap_time;

  SEND(0, USEC(10), &tg, setTone, getPeriods(frequencies[i]));
  SEND(play_time, gap_time, &tg, silence, 0);d
  self->index = (self->index + 1) % 32;
  SEND(next_start, next_stop, &mp, playNote, 0);
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
