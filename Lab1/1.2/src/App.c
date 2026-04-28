#include "App.h"
#include "TinyTimber.h"
#include "canTinyTimber.h"
#include "sciTinyTimber.h"

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
    ASYNC(self, increaseLoad, 0);
  }
  else if (c == 'd') {
    ASYNC(self, decreaseLoad, 0);
  }
  else {
    return;
  }
}

void toggleMute(App *self) {
  self->muted = !self->muted;
  SCI_WRITE(&sci0, self->muted == 0 ? "Unmuted\n" : "Muted\n");
}

void volumeUp(App *self) {
  if (self->volume < 20) {
    self->volume++;
  }
  // Print
  char buff[32];
  snprintf(buff, sizeof(buff), "Volume: %d\n", self->volume);
  SCI_WRITE(&sci0, buff);
}

void volumeDown(App *self) {
  if (self->volume > 0) {
    self->volume--;
  }
  // Print
  char buff[32];
  snprintf(buff, sizeof(buff), "Volume: %d\n", self->volume);
  SCI_WRITE(&sci0, buff);
}

void toneGenerator(App *self) {
  if (!togglestate) {
    *DAC_DATA = 0;
    togglestate = 1;
  } else {
    *DAC_DATA = self->muted ? 0 : self->volume;
    togglestate = 0;
  }

  SEND(USEC(500), 0, self, toneGenerator, 0);
  // SEND ger både baseline och deadline argument till ASYNC
  // USEC(500) tillser att 500us konverteras korrekt till processorns "tidsenheter"
}

void backgroundLoad(BackgroundTask *self) {
  for (int i = 0; i < self->background_loop_range; i++) {}

  SEND(USEC(1300), 0, self, backgroundLoad, 0);
}

void increaseLoad(BackgroundTask *self) {
  self->background_loop_range += 500;

  // Print
  char buff[32];
  snprintf(buff, sizeof(buff), "load: %d\n", self->background_loop_range);
  SCI_WRITE(&sci0, buff);
}

void decreaseLoad(BackgroundTask *self) {
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

  ASYNC(&backgroundTask, backgroundLoad, 0)
}

int main() {
  INSTALL(&sci0, sci_interrupt, SCI_IRQ0);
  INSTALL(&can0, can_interrupt, CAN_IRQ0);
  TINYTIMBER(&app, startApp, 0);
  return 0;
}
