#include "App.h"
#include "TinyTimber.h"
#include "canTinyTimber.h"
#include "sciTinyTimber.h"
#include <stdio.h>

/* == GLOBALA OBJEKT == */
extern App app;
extern Can can0;
extern Serial sci0;
extern BackgroundTask backtask;

/* == STATISKA VARIABLER == */
static int togglestate = 0;

/* == I/O FUNKTIONER == */
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
    switch (c) {
        case 'm': ASYNC(self, toggleMute, 0);                 break; // MUTE

        case '+': ASYNC(self, volumeUp,   0);                 break; // ÖKA VOLYM

        case '-': ASYNC(self, volumeDown, 0);                 break; // SÄNK VOLYM

        case 'u': ASYNC(&backtask, increaseLoad, 0);          break; // ÖKA DIST

        case 'd': ASYNC(&backtask, decreaseLoad, 0);          break; // SÄNK DIST
        case '1': setFrequency(self, 1000);

                  SCI_WRITE(&sci0, "Frequency: 1000 Hz\n");   break; // FREQ 1
        case '2': setFrequency(self, 769);

                  SCI_WRITE(&sci0, "Frequency: 769 Hz\n");    break; // FREQ 2
        case '3': setFrequency(self, 537);

                  SCI_WRITE(&sci0, "Frequency: 537 Hz\n");    break; // FREQ 3
        case 'j':
                  if (self->deadline) {
                    SCI_WRITE(&sci0, "Deadline activating\n");
                  } else {
                    SCI_WRITE(&sci0, "Deadline deactivating\n");
                  }
                  ASYNC(self,      toggleDeadline, 0);
                  ASYNC(&backtask, toggleDeadlineBG, 0);        break; // TOGGLE DIST
        default:
            break;
    }
}

/* == DEADLINE KONTROLL == */
void toggleDeadline(App *self, int arg) {
  self->deadline = !self->deadline;
}

void toggleDeadlineBG(BackgroundTask *self, int arg) {
  self->deadline = !self->deadline;
}

/* == VOLYM KONTROLL == */
void toggleMute(App *self, int arg) {
  self->muted = !self->muted;
  if (self->muted) {
    SCI_WRITE(&sci0, "Muted\n");
  } else {
    SCI_WRITE(&sci0, "Unmuted\n");
  }
}

void volumeUp(App *self, int arg) {
  if (self->volume < 20) self->volume++;

  char buff[32];
  snprintf(buff, sizeof(buff), "Volume: %d\n", self->volume);
  SCI_WRITE(&sci0, buff);
}

void volumeDown(App *self, int arg) {
  if (self->volume > 0) self->volume--;

  char buff[32];
  snprintf(buff, sizeof(buff), "Volume: %d\n", self->volume);
  SCI_WRITE(&sci0, buff);
}

/* == TON KONTROLL == */
void setFrequency(App *self, int freq){
  self->period_us = (1000000 / (2 * (freq)));
}

void toneGenerator(App *self, int arg) {
  /* -- Variabler för hantering av tiden -- */
  static Time max_wcet = 0;
  static long long sum_wcet = 0;
  static int count = 0;

  Time start = CURRENT_OFFSET(); // Starta tidtagning

  /* -- kör funktionen -- */
  if (!togglestate) {
    *DAC_DATA = 0;
    togglestate = 1;
  } else {
    *DAC_DATA = self->muted ? 0 : self->volume;
    togglestate = 0;
  }

  Time elapsed = CURRENT_OFFSET() - start; // Stoppa tidtagning

  /* -- Hantera datainsamling -- */
  if (count < 500) {
    if (elapsed > max_wcet) max_wcet = elapsed;
    sum_wcet += elapsed;
    count++;
  }

  /* -- Hantera resultat -- */
  else if (count == 500) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer),
    "--- Time ---\nMax: %ld us\nAvg: %ld us\n",
    ((long)max_wcet * 10),
    (long)((sum_wcet / 500) * 10));
    SCI_WRITE(&sci0, buffer);
    count++;
  }

  SEND(USEC(self->period_us), self->deadline == 1 ? USEC(100) : 0, self, toneGenerator, 0);
  // SEND ger både baseline och deadline argument till ASYNC
  // USEC(500) tillser att 500us konverteras korrekt till processorns "tidsenheter"
}

/* == DISTORTION == */
void backgroundLoad(BackgroundTask *self, int arg) {
  for (int i = 0; i < self->background_loop_range; i++) {}

  SEND(USEC(1300), self->deadline == 1 ? USEC(1300) : 0, self, backgroundLoad, 0);  // Deadline = arg 2
}

void increaseLoad(BackgroundTask *self, int arg) {
  self->background_loop_range += 500;

  char buff[32];
  snprintf(buff, sizeof(buff), "load: %d\n", self->background_loop_range);
  SCI_WRITE(&sci0, buff);
}

void decreaseLoad(BackgroundTask *self, int arg) {
  if (self->background_loop_range > 500) self->background_loop_range -= 500;

  char buff[32];
  snprintf(buff, sizeof(buff), "load: %d\n", self->background_loop_range);
  SCI_WRITE(&sci0, buff);
}

/* == PROGRAM KONTROLL == */
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
  // ASYNC(&backtask, backgroundLoad, 0);
}

int main() {
  INSTALL(&sci0, sci_interrupt, SCI_IRQ0);
  INSTALL(&can0, can_interrupt, CAN_IRQ0);
  TINYTIMBER(&app, startApp, 0);
  return 0;
}
