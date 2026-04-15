#include "App.h"
#include "string.h"
#include "TinyTimber.h"
#include "canTinyTimber.h"
#include "sciTinyTimber.h"

extern App app;
/* FROM APP_H
typedef struct {
  Object super;
  int history[3];
  int history_index;
  char buffer[20];
  int cnt;
} App;
*/
extern Can can0;
extern Serial sci0;

void receiver(App *self, int unused) {
  CANMsg msg;
  CAN_RECEIVE(&can0, &msg);
  SCI_WRITE(&sci0, "Can msg received: ");
  SCI_WRITE(&sci0, msg.buff);
}

int Sum(App *self) {
  switch (self->history_index) {
  case 1: return self->history[2];
  case 2: return self->history[2] + self->history[1];
  case 3: return self->history[2] + self->history[1] + self->history[0];
  
  default: return 0;
  }
}

int Median(App *self){
  if (self->history_index == 1) return self->history[2];                          // Om endast ett tal, returnera det
  else if (self->history_index == 2) return (self->history[1] + self->history[2]) / 2; // Om två tal, returnera medelvärde
  else {                        
    int a = self->history[0]; // Spara för enklare läs/skriv
    int b = self->history[1];
    int c = self->history[2];
    
    if ((a>=b && a<=c)||(a>=c && a<=b)) return a;       // Om a är i mitten, returnera a
    else if ((b>=a && b<=c)||(b>=c && b<=a)) return b;  // Om b är i mitten, returnera b
    else return c;                                      // Annars returnera c
  }
}

void WriteInteger(int a) {
  char out[16];
  snprintf(out, sizeof(out), "%d", a);
  SCI_WRITE(&sci0, out);
}

void reader(App *self, int c) {
  if (c == 'F') {                                       /* OM HISTORIK SKA RENSAS */
    self->history_index = 0;                            // Nollställ index
    memset(self->history, 0, sizeof(self->history));    // Rensa history
    memset(self->buffer, 0, sizeof(self->buffer));      // Rensa buffer
    self->cnt = 0;                                      // Nollställ cnt
    SCI_WRITE(&sci0, "The 3-history has been erased\n");
  } else if (c != 'e' && self->cnt < 19) {              /* OM INTEGER EJ ÄR FÄRDIG */
    self->buffer[self->cnt] = c;                        // Spara mottagen char i buffer
    self->cnt++;                                        // Inkrementera cnt
    SCI_WRITECHAR(&sci0, c);                            // Skriv ut nuvarande char                           
  } else {                                              /* OM INTEGER ÄR FÄRDIG */
    self->buffer[self->cnt] = '\0';                     // EOS
    int num = atoi(self->buffer);                       // Konvertera till integer
    self->cnt = 0;                                      // Nollställ cnt
    memset(self->buffer, 0, sizeof(self->buffer));      // Rensa buffer
    
    self->history[0] = self->history[1];
    self->history[1] = self->history[2];
    self->history[2] = num;

    if (self->history_index < 3)
        self->history_index++;
    
    int s = Sum(self);
    int m = Median(self);

    SCI_WRITE(&sci0, "Entered integer ");
    WriteInteger(num);
    SCI_WRITE(&sci0, " sum = ");
    WriteInteger(s);
    SCI_WRITE(&sci0, " median = ");
    WriteInteger(m);
    SCI_WRITECHAR(&sci0, '\n');
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
