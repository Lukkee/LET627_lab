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
  static int cnt = 0;             // Initiera räknare och int
  static char buffer[20];         // Skapa buffer
  if (c != 'e' && cnt < 19) {     // Medan escape char inte skickats och buffern fortfarande har plats
    buffer[cnt] = c;              // Spara char i buffer
    cnt++;                        // Inkrementera räknare
    SCI_WRITECHAR(&sci0, c);      // Skriv char i terminal
  } else {
      buffer[cnt] = '\0';                           // Stäng sträng
      int num = atoi(buffer);                       // Omvandla sträng till int
      char out[30];
      snprintf(out, sizeof(out), "Number: %d", num);// Skriv ut int i terminal
      SCI_WRITE(&sci0, out);                        // Skriv ut sträng i terminal
      memset(buffer, 0, sizeof(buffer));            // Nollställ buffer
      cnt = 0;                                      // Nollställ räknare
  }
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
