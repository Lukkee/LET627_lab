#include "App.h"
#include "TinyTimber.h"
#include "canTinyTimber.h"
#include "sciTinyTimber.h"
#include "sioTinyTimber.h"
#include "stm32f4xx_tim.h"  // timer
#include <stdlib.h>
#include <stdio.h>

extern App app;
extern Can can0;
extern Serial sci0;
extern SysIO sio0;

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

void checkHold(App *self, int arg) {
  if (self->pressed) {
    self->mode = 1;
    SCI_WRITE(&sci0, "Entered PRESS-TO-HOLD\n");
  }
}

void SioCallback(App *self, int arg) {
  self->now = TIM_GetCounter(TIM5) / 100;
  Time diff = self->now - self->last;
  char buffer[32];
  if (diff < 10) return;
  if (!self->pressed) {
    SIO_TRIG(&sio0, 1);
    self->pressed = 1;
    self->pending = AFTER(SEC(1), self, checkHold, 0);
  } else {
    SIO_TRIG(&sio0, 0);
    self->pressed = 0;
    if (self->mode) { /* OM HOLD */
      self->mode = 0;
      snprintf(buffer, sizeof(buffer), "Held for: %dms\n", (int)diff);
      SCI_WRITE(&sci0, buffer);
    } else {          /* OM MOMENTARY */
      SCI_WRITE(&sci0, "MOMENTARY-PRESS\n");
    }
    ABORT(self->pending);
  }
  self->last = self->now;
}

void startApp(App *self, int arg) {
  CANMsg msg;

  CAN_INIT(&can0);
  SIO_INIT(&sio0);
  SIO_TRIG(&sio0, 1);
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

void EXTI9_5_IRQHandler(void) {
  sio_interrupt(&sio0, 0);
}

int main() {
  INSTALL(&sci0, sci_interrupt, SCI_IRQ0);
  INSTALL(&can0, can_interrupt, CAN_IRQ0);
  INSTALL(&sio0, sio_interrupt, SIO_IRQ0);
  TINYTIMBER(&app, startApp, 0);
  return 0;
}
