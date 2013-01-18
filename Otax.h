#ifndef OTAX_H
#define OTAX_H

////RX 434MHz digital pin3
/* Decoder for 433 MHz OTAX thermostat
S=800us   L=1450us
boris ON  SLL SLL SLL SSLS     DB2 répété 10 fois, puis DB6
boris OFF SLL SLL SLL SLSS     DB4 répété 10 fois, puis DB6
maman ON  SLL LLL SLL SSLS     FB2 répété 10 fois, puis FB6
maman OFF SLL LLL SLL SLSS     FB4 répété 10 fois, puis FB6
apparemment une fin de msg est signalée avec DB6 (après on et après off)
1 bit start (S)
5 bits id chaudiere (maman: LLLLL boris: LLSLL)
3 bits commande ??? tjours SLL
4 bits action demandée SSLS=on  SLSS=off
*/

#include <inttypes.h>
#include "pins.h"

enum { UNKNOWN, OK, DONE };
enum { OTAX_PROXY, OTAX_NOPROXY };
enum { OTAX_NOTRECEIVED, OTAX_RXOFF, OTAX_RXON };
enum { OTAX_DONTFORCE, OTAX_FORCEOFF, OTAX_FORCEON, OTAX_REGULATE };

class Otax {
  public:
    uint8_t profile; // = OTAX_PROXY;
    uint8_t rx; // = OTAX_NOTRECEIVED;
    uint8_t force; // = OTAX_DONTFORCE;
    unsigned long lastTXtime; // = 0; // dernier TX vers l'OTAX (ms)
    unsigned long lastRXtime; // = 0; // dernier RX de la télécommande OTAX (ms)

      Otax();
      void sendOn();
      void sendOff();
      uint8_t state;
      char bits;
      uint16_t data;
      uint8_t OTAX_bit(uint8_t value);

  private:
      void short0();
      void long1();
};

#endif

