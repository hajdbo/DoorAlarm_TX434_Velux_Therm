#ifndef POWERTX434_H
#define POWERTX434_H

//TX 434MHz pour powerOn/Off digital pin7
// codepowerDevice (8bits) puis codePWR (17 bits)
// puis 7 ms. répéter 20 fois (600 ms)
// ON:  8S 2L 7S
// OFF: 17S
// 1: 2S 6L
// 2: 4S 4L
// 3: 2S 2L 2S 2L
// 4: 6S 2L
// 5: 2S 4L 2S

#include "pins.h"

class PowerTX434 {
public:
    //static const int TXpin = 7;
    static void powerOn (int device);
    static void powerOff(int device);

private:
    static void powerShort0();
    static void powerLong1();
    static void powerON();
    static void powerOFF();
    static void powerDevice1();
    static void powerDevice2();
    static void powerDevice3();
    static void powerDevice4();
    static void powerDevice5();
};

#endif

