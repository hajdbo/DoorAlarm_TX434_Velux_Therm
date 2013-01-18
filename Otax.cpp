#include "Arduino.h"
#include "Otax.h"
#include  <digitalWriteFast.h>

//struct { uint8_t state; char bits; uint16_t data; } OTAX;
//struct { uint8_t state; char bits; uint16_t data; } Otax;

// constructeur
Otax::Otax() {
  OTAXprofile = OTAX_PROXY;
  OTAXrx = OTAX_NOTRECEIVED;
  OTAXforce = OTAX_DONTFORCE;
  OTAXlastTXtime = 0; // dernier TX vers l'OTAX (ms)
  OTAXlastRXtime = 0; // dernier RX de la télécommande OTAX (ms)
}

void Otax::OTAXon() {
  for (int count=0; count<10; count++) {
    cli();
    for (int i=0; i<3; i++) { OTAXshort0(); OTAXlong1(); OTAXlong1(); } //adresse chaudiere de boris
    OTAXshort0();
    OTAXshort0();
    OTAXlong1();
    OTAXshort0();  
    digitalWriteFast(TX434pin, LOW);
    sei();
    delay(10);
  }
}

void Otax::OTAXoff() {
  for (int count=0; count<10; count++) {
    cli();
    for (int i=0; i<3; i++) { OTAXshort0(); OTAXlong1(); OTAXlong1(); } //adresse chaudiere de boris
    OTAXshort0();
    OTAXlong1();
    OTAXshort0();
    OTAXshort0();
    digitalWriteFast(TX434pin, LOW);
    sei();
    delay(10);
  }
}

/// PRIVATE METHODS ///////////:
void Otax::OTAXshort0() {
  digitalWriteFast(TX434pin, LOW);
  delayMicroseconds(1450);
  digitalWriteFast(TX434pin, HIGH);
  delayMicroseconds(800);
}

void Otax::OTAXlong1() {
  digitalWriteFast(TX434pin, LOW);
  delayMicroseconds(800);
  digitalWriteFast(TX434pin, HIGH);
  delayMicroseconds(1450);
}

uint8_t Otax::OTAX_bit(uint8_t value) {
    data = (data << 1) | value;
    return ++bits != 13 ? OK : DONE;
}

