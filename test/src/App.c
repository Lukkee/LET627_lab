#include "App.h"
#include "string.h"
#include "TinyTimber.h"
#include "canTinyTimber.h"
#include "sciTinyTimber.h"

extern App app;
extern Can can0;
extern Serial sci0;

void receiver(App *self, int unused) {
  CANMsg msg;
  CAN_RECEIVE(&can0, &msg);
  SCI_WRITE(&sci0, "Can msg received: ");
  SCI_WRITE(&sci0, msg.buff);
}

void reader(App *self, int c) {
  SCI_WRITE(&sci0, "Rcv: \'");    // Börja skriv
  int cnt = 0, num;               // Initiera räknare och int
  char buffer[20];                // Skapa buffer
  while (c != 'e' && cnt < 20) {  // Medan escape char inte skickats och buffern fortfarande har plats
    buffer[cnt] = c;              // Spara char i buffer
    cnt++;                        // Inkrementera räknare
    SCI_WRITECHAR(&sci0, c);      // Skriv char i terminal
  }
  buffer[cnt + 1] = '\0';         // Stäng sträng
  num = atoi(buffer);             // Omvandla sträng till int
  SCI_WRITE(&sci0, "\'\n");       // Avsluta skriv
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
