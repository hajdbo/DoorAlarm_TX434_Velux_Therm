#ifndef OTAX_H
#define OTAX_H

#include <inttypes.h>

//struct { uint8_t state; char bits; uint16_t data; } OTAX;
//struct { uint8_t state; char bits; uint16_t data; } Otax;
enum { UNKNOWN, OK, DONE };


//uint8_t OTAX_bit(uint8_t value);
void OTAXoff();
void OTAXon();
void OTAXshort0();
void OTAXlong1();
#define TX434pin 7
//extern const int TXpin;


/*class Otax {
public:
    static const int TXpin = 7;
    static void OTAXon();
    static void OTAXoff();
    static uint8_t state;
    static char bits;
    static uint16_t data;

private:
    static void OTAXshort0();
    static void OTAXlong1();
    
};*/

#endif

