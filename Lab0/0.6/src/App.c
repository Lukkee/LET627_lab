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

/* FRÅN APP_H
typedef struct {
  Object super;
  int history[3];
  int history_index;
  char buffer[12];
  int cnt;
} App;
*/

int Sum(App *self) {
  switch (self->history_index) {
  case 1: return self->history[2];
  case 2: return (self->history[1] + self->history[2]);
  case 3: return (self->history[0] + self->history[1] + self->history[2]);

  default: return 0;
  }
}

void KeyToPeriods(App *self, int key) {

  // Om key är utanför [-5, 5]
  if (key < -5 || key > 5) {
    SCI_WRITE(&sci0, "Key must be within [-5, 5]\n");
    return;
  }

  double periods_from_key[32];

  char buffer[32];
  snprintf(buffer, sizeof(buffer), "Key: %d ", key);
  SCI_WRITE(&sci0, buffer);

  for (int i = 0; i < 32; i++) {
    int indice = self->indices[i] + key;
    int period = self->periods[indice + 10];
    periods_from_key[i] = period;

    snprintf(buffer, sizeof(buffer), " %d", period);
    SCI_WRITE(&sci0, buffer);
  }

  SCI_WRITE(&sci0, "\n");

  /*
  Print out the periods as follows: First print ”Key: ” followed by the chosen
  key value and a line break. Then print the 32 period values on one line, each
  value separated by a space character, and terminate with a line break.
  */

}

int Median(App *self) {
  if (self->history_index == 1) return self->history[2];    // Enskilt tal
  else if (self->history_index == 2) return (self->history[2] + self->history[1]) / 2;  // Medelvärde
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
  char out[12];
  snprintf(out, sizeof(out), "%d", a);
  SCI_WRITE(&sci0, out);
}

void receiver(App *self, int unused) {
  CANMsg msg;
  CAN_RECEIVE(&can0, &msg);
  SCI_WRITE(&sci0, "Can msg received: ");
  SCI_WRITE(&sci0, msg.buff);
}


  // RECIEVE

void reader(App *self, int c) {
  SCI_WRITE(&sci0, "Rcv: \'");
  SCI_WRITECHAR(&sci0, c);
  SCI_WRITE(&sci0, "\'\n");

  if ( c == 'F' ) { /* RENSA HISTORIK */
    self->history_index = 0;                          // Nollställ history_index
    memset(self->history, 0, sizeof(self->history));  // Rensa history
    memset(self->buffer, 0, sizeof(self->buffer));    // Rensa buffer
    self->cnt = 0;                                    // Nollställ cnt
    SCI_WRITE(&sci0, "The 3-history has been erased\n");
  }
  else if ( c != 'e' && self->cnt < 11) { /* FORTSÄTT SKRIVA */
    self->buffer[self->cnt] = c;    // spara char till buffer[cnt]
    self->cnt++;                    // öka cnt
  }
  else { /* SLUTA SKRIVA */
    self->buffer[self->cnt] = '\0';                 // null-terminator
    int num = atoi(self->buffer);                   // ASCII to Integer(buffer)
    self->cnt = 0;                                  // Nollställ cnt
    memset(self->buffer, 0, sizeof(self->buffer));  // Rensa buffer

    self->history[0] = self->history[1];
    self->history[1] = self->history[2];  // Flytta bak rest
    self->history[2] = num;               // Nytt värde i [2]

    if (self->history_index < 3) self->history_index++;   // Håll koll på första tre för särskilda fall i median och summa

    KeyToPeriods(self, num);

    /* SKRIV UT */
//    int s = Sum(self);    // Hämta summan
//    int m = Median(self); // Hämta medianen
//
//    SCI_WRITE(&sci0, "Entered integer ");
//    WriteInt(num);
//    SCI_WRITE(&sci0, ": sum = ");
//    WriteInt(s);
//    SCI_WRITE(&sci0, ", median = ");
//    WriteInt(m);
//    SCI_WRITE(&sci0, "\n");
  }
}

void startApp(App *self, int arg) {
  CANMsg msg;

  int tmp_indice[] = {0, 2, 4, 0, 0, 2, 4, 0, 4, 5, 7, 4, 5, 7, 7, 9, 7, 5, 4, 0, 7, 9, 7, 5, 4, 0, 0, -5, 0, 0, -5, 0};
  memcpy(self->indices, tmp_indice, sizeof(tmp_indice));
  int tmp_periods[] = {
                          2025,
                          1911,
                          1803,
                          1702,
                          1607,
                          1516,
                          1431,
                          1351,
                          1275,
                          1203,
                          1136,
                          1072,
                          1012,
                          955,
                          901,
                          851,
                          803,
                          758,
                          715,
                          675,
                          637,
                          601,
                          568,
                          536,
                          506
                          };
  memcpy(self->periods, tmp_periods, sizeof(tmp_periods));

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
