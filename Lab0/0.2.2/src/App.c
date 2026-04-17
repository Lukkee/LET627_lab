#include "App.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "TinyTimber.h"
#include "canTinyTimber.h"
#include "sciTinyTimber.h"

extern App app;
extern Can can0;
extern Serial sci0;

int Sum(App *self) {
  switch (self->history_index) {
  case 1: return self->history[2];
  case 2: return (self->history[1] + self->history[2]);
  case 3: return (self->history[0] + self->history[1] + self->history[2]);

  default: return 0;
  }
}

int Median(App *self) {
  if (self->history_index == 1) return self->history[2];
  else if (self->history_index == 2) return (self->history[2] + self->history[1]) / 2;
  else {
    int a = self->history[0]; // Spara för enklare läs/skriv
    int b = self->history[1];
    int c = self->history[2];

    if ((a>=b && a<=c)||(a>=c && a<=b)) return a;       // Om a är i mitten, returnera a
    else if ((b>=a && b<=c)||(b>=c && b<=a)) return b;  // Om b är i mitten, returnera b
    else return c;
  }
}

void WriteInt( int a ) {
  char out[20];
  snprintf(out, sizeof(out), "%d", a);
  SCI_WRITE(&sci0, out);
}

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

  if ( c == 'F' ) { /* RENSA HISTORIK */
    self->history_index = 0;
    memset(self->history, 0, sizeof(self->history));
    memset(self->buffer, 0, sizeof(self->buffer));
    self->cnt = 0;
    SCI_WRITE(&sci0, "The 3-history has been erased\n");
  }
  else if ( c != 'e' && self->cnt < 19) { /* FORTSÄTT SKRIVA */
    self->buffer[self->cnt] = c;
    self->cnt++;
  }
  else { /* SLUTA SKRIVA */
    self->buffer[self->cnt] = '\0';
    int num = atoi(self->buffer);
    self->cnt = 0;
    memset(self->buffer, 0, sizeof(self->buffer));

    self->history[0] = self->history[1];
    self->history[1] = self->history[2];
    self->history[2] = num;

    if (self->history_index < 3) self->history_index++;


    /* SKRIV UT */
    int s = Sum(self);
    int m = Median(self);

    SCI_WRITE(&sci0, "Entered integer ");
    WriteInt(num);
    SCI_WRITE(&sci0, ": sum = ");
    WriteInt(s);
    SCI_WRITE(&sci0, ", median = ");
    WriteInt(m);
    SCI_WRITE(&sci0, "\n");
  }
}

void startApp(App *self, int arg) {
  CANMsg msg;

  CAN_INIT(&can0);
  SCI_INIT(&sci0);
  SCI_WRITE(&sci0, "Hello, woooorld!\n");

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
