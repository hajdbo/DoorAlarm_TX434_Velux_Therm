#include "Arduino.h"
#include "Otax.h"
#include  <digitalWriteFast.h>

Otax::Otax() {
  profile = OTAX_PROXY;
  rx = OTAX_NOTRECEIVED;
  force = OTAX_DONTFORCE;
  lastTXtime = 0; // dernier TX vers l'OTAX (ms)
  lastRXtime = 0; // dernier RX de la télécommande OTAX (ms)
}

void Otax::sendOn() {
  for (int count=0; count<10; count++) {
    cli();
    for (int i=0; i<3; i++) { short0(); long1(); long1(); } //adresse chaudiere de boris
    short0();
    short0();
    long1();
    short0();  
    digitalWriteFast(TX434pin, LOW);
    sei();
    delay(10);
  }
}

void Otax::sendOff() {
  for (int count=0; count<10; count++) {
    cli();
    for (int i=0; i<3; i++) { short0(); long1(); long1(); } //adresse chaudiere de boris
    short0();
    long1();
    short0();
    short0();
    digitalWriteFast(TX434pin, LOW);
    sei();
    delay(10);
  }
}

/// PRIVATE METHODS ///////////:
void Otax::short0() {
  digitalWriteFast(TX434pin, LOW);
  delayMicroseconds(1450);
  digitalWriteFast(TX434pin, HIGH);
  delayMicroseconds(800);
}

void Otax::long1() {
  digitalWriteFast(TX434pin, LOW);
  delayMicroseconds(800);
  digitalWriteFast(TX434pin, HIGH);
  delayMicroseconds(1450);
}

uint8_t Otax::OTAX_bit(uint8_t value) {
    data = (data << 1) | value;
    return ++bits != 13 ? OK : DONE;
}

