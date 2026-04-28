#include "App.h"
#include "TinyTimber.h"
#include "canTinyTimber.h"
#include "sciTinyTimber.h"
#include <stdio.h>

extern App app;
extern Can can0;
extern Serial sci0;
extern BackgroundTask backtask;

/* FRÅN APP_H
  typedef struct {
    Object super;
    int history[3];
    int history_index;
    char buffer[12];
    int cnt;
    int volume;
    int muted;
  } App;
*/

static int togglestate = 0;

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
  if (c == 'm') {
    ASYNC(self, toggleMute, 0);
  }
  else if (c == '+') {
    ASYNC(self, volumeUp, 0);
  }
  else if (c == '-') {
    ASYNC(self, volumeDown, 0);
  }
  else if (c == 'u') {
    ASYNC(&backtask, increaseLoad, 0);
  }
  else if (c == 'd') {
    ASYNC(&backtask, decreaseLoad, 0);
  }
  else if (c == '1') {
    setFrequency(self, 1000);
    SCI_WRITE(&sci0, "Frequency: 1kHz\n");
  }
  else if (c == '2') {
    setFrequency(self, 769);
    SCI_WRITE(&sci0, "Frequency: 769Hz\n");
  }
  else if (c == '3') {
    setFrequency(self, 537);
    SCI_WRITE(&sci0, "Frequency: 537Hz\n");
  }
  else if (c == 'j') {
    ASYNC(self, toggleDeadline, 0);
    ASYNC(&backtask, toggleDeadline, 0);
    if (self->deadline == 1) {
      SCI_WRITE(&sci0, "Deadline Deactivated\n");
    } else {
      SCI_WRITE(&sci0, "Deadline Activated\n");
    }
  }
  else {
    return;
  }
}

void toggleDeadline(App *self, int arg) {
  self->deadline = self->deadline == 0 ? 1 : 0;
}

void setFrequency(App *self, int freq){
  self->period_us = (1000000 / (2 * (freq)));
}

void toggleMute(App *self, int arg) {
  self->muted = !self->muted;
  if (self->muted == 0) {
    SCI_WRITE(&sci0, "Unmuted\n");
  } else {
    SCI_WRITE(&sci0, "Muted\n");
  }
}

void volumeUp(App *self, int arg) {
  if (self->volume < 20) {
    self->volume++;
  }
  // Print
  char buff[32];
  snprintf(buff, sizeof(buff), "Volume: %d\n", self->volume);
  SCI_WRITE(&sci0, buff);
}

void volumeDown(App *self, int arg) {
  if (self->volume > 0) {
    self->volume--;
  }
  // Print
  char buff[32];
  snprintf(buff, sizeof(buff), "Volume: %d\n", self->volume);
  SCI_WRITE(&sci0, buff);
}

void toneGenerator(App *self, int arg) {
  if (!togglestate) {
    *DAC_DATA = 0;
    togglestate = 1;
  } else {
    *DAC_DATA = self->muted ? 0 : self->volume;
    togglestate = 0;
  }

  SEND(USEC(self->period_us), self->deadline == 1 ? USEC(100) : 0, self, toneGenerator, 0);
  // SEND ger både baseline och deadline argument till ASYNC
  // USEC(500) tillser att 500us konverteras korrekt till processorns "tidsenheter"
}

void backgroundLoad(BackgroundTask *self, int arg) {
  for (int i = 0; i < self->background_loop_range; i++) {}

  SEND(USEC(1300), self->deadline == 1 ? USEC(1300) : 0, self, backgroundLoad, 0);  // Deadline = arg 2
}

void increaseLoad(BackgroundTask *self, int arg) {
  self->background_loop_range += 500;

  // Print
  char buff[32];
  snprintf(buff, sizeof(buff), "load: %d\n", self->background_loop_range);
  SCI_WRITE(&sci0, buff);
}

void decreaseLoad(BackgroundTask *self, int arg) {
  if (self->background_loop_range > 500) self->background_loop_range -= 500;

  // Print
  char buff[32];
  snprintf(buff, sizeof(buff), "load: %d\n", self->background_loop_range);
  SCI_WRITE(&sci0, buff);
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

  self->volume = 3;
  self->period_us = 500;

  ASYNC(self, toneGenerator, 0);
  ASYNC(&backtask, backgroundLoad, 0);
}

int main() {
  INSTALL(&sci0, sci_interrupt, SCI_IRQ0);
  INSTALL(&can0, can_interrupt, CAN_IRQ0);
  TINYTIMBER(&app, startApp, 0);
  return 0;
}
