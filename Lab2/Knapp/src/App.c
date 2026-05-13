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
  if (self->pressed) {                    // Om pressed fortfarande är igång
    self->mode = 1;                       // Sätt mode
    SCI_WRITE(&sci0, "Entered PRESS-TO-HOLD\n");
  }
}


void SioCallback(App *self, int arg) {
  Time since_last = T_SAMPLE(&self->timer);
  if (since_last < MSEC(100)) {
    return; // Filtrera undan contact bounces
  }

  char buffer[64];

  if (!self->pressed) {
    self->pressed = 1;
    T_RESET(&self->press_timer);  // Start measuring hold duration
    self->pending = AFTER(SEC(1), self, checkHold, 0);
    SIO_TRIG(&sio0, 1);
  } else {
    int diff_ms = since_last / 100;           // Inter-press interval
    int hold_ms = T_SAMPLE(&self->press_timer) / 100;  // Hold duration
    self->pressed = 0;
    T_RESET(&self->timer);
    ABORT(self->pending);
    SIO_TRIG(&sio0, 0);

    if (self->mode) {                       // Om checkHold har gått igenom
      self->mode = 0;
      snprintf(buffer, sizeof(buffer), "Held for: %ds\n", (int)hold_ms / 1000);
      SCI_WRITE(&sci0, buffer);
    } else {                                      // Om checkHold ej gått igenom
      if (diff_ms < 3000) {                  // Om intervallet inte för högt
        snprintf(buffer, sizeof(buffer), "Interval: %dms\n", (int)diff_ms);
        SCI_WRITE(&sci0, buffer);
      }
      }
    }
}

void startApp(App *self, int arg) {
  CAN_INIT(&can0);
  SIO_INIT(&sio0);
  SIO_TRIG(&sio0, 1);
  SCI_INIT(&sci0);
  T_RESET(&self->press_timer);

  SCI_WRITE(&sci0, "Hello, hello...\n");
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
