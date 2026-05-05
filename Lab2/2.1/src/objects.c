#include "App.h"
#include "canTinyTimber.h"
#include "sciTinyTimber.h"

App app = initApp();
MusicPlayer mp = initMusicPlayer();
ToneGenerator tg = initToneGenerator();
Can can0 = initCan(CAN_PORT0, &app, receiver);
Serial sci0 = initSerial(SCI_PORT0, &app, reader);
