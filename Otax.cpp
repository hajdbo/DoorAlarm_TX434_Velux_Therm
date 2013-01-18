#include "Arduino.h"
#include "Otax.h"
#include  <digitalWriteFast.h>

//struct { uint8_t state; char bits; uint16_t data; } OTAX;
//struct { uint8_t state; char bits; uint16_t data; } Otax;


void OTAXon() {
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

void OTAXoff() {
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
void OTAXshort0() {
  digitalWriteFast(TX434pin, LOW);
  delayMicroseconds(1450);
  digitalWriteFast(TX434pin, HIGH);
  delayMicroseconds(800);
}

void OTAXlong1() {
  digitalWriteFast(TX434pin, LOW);
  delayMicroseconds(800);
  digitalWriteFast(TX434pin, HIGH);
  delayMicroseconds(1450);
}



/*
void Otax::OTAXon() {
  for (int count=0; count<10; count++) {
    for (int i=0; i<3; i++) { OTAXshort0(); OTAXlong1(); OTAXlong1(); } //adresse chaudiere de boris
    OTAXshort0();
    OTAXshort0();
    OTAXlong1();
    OTAXshort0();  
    digitalWrite(TXpin, LOW);
    delay(10);
  }
}

void Otax::OTAXoff() {
  for (int count=0; count<10; count++) {
    for (int i=0; i<3; i++) { OTAXshort0(); OTAXlong1(); OTAXlong1(); } //adresse chaudiere de boris
    OTAXshort0();
    OTAXlong1();
    OTAXshort0();
    OTAXshort0();
    digitalWrite(TXpin, LOW);
    delay(10);
  }
}

/// PRIVATE METHODS ///////////:
void Otax::OTAXshort0() {
  digitalWrite(TXpin, LOW);
  delayMicroseconds(1450);
  digitalWrite(TXpin, HIGH);
  delayMicroseconds(800);
}

void Otax::OTAXlong1() {
  digitalWrite(TXpin, LOW);
  delayMicroseconds(800);
  digitalWrite(TXpin, HIGH);
  delayMicroseconds(1450);
}
*/
