#ifndef POWERTX434_H
#define POWERTX434_H


class PowerTX434 {
public:
    static const int TXpin = 7;
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

