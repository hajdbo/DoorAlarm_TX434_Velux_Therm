#include "Arduino.h"
#include "PowerTX434.h"
#include  <digitalWriteFast.h>

void PowerTX434::powerOn (int device) {
  switch(device) {
    case 1: for (int i=0; i<20; i++) { cli(); powerDevice1(); powerON();  sei(); delay(7); } delay(1000); break;
    case 2: for (int i=0; i<20; i++) { cli(); powerDevice2(); powerON();  sei(); delay(7); } delay(1000); break;
    case 3: for (int i=0; i<20; i++) { cli(); powerDevice3(); powerON();  sei(); delay(7); } delay(1000); break;
    case 4: for (int i=0; i<20; i++) { cli(); powerDevice4(); powerON();  sei(); delay(7); } delay(1000); break;
    case 5: for (int i=0; i<20; i++) { cli(); powerDevice5(); powerON();  sei(); delay(7); } delay(1000); break;
  }
}
    
void PowerTX434::powerOff(int device) {
  switch(device) {
    case 1: for (int i=0; i<20; i++) { cli(); powerDevice1(); powerOFF();  sei(); delay(7); } delay(1000); break;
    case 2: for (int i=0; i<20; i++) { cli(); powerDevice2(); powerOFF();  sei(); delay(7); } delay(1000); break;
    case 3: for (int i=0; i<20; i++) { cli(); powerDevice3(); powerOFF();  sei(); delay(7); } delay(1000); break;
    case 4: for (int i=0; i<20; i++) { cli(); powerDevice4(); powerOFF();  sei(); delay(7); } delay(1000); break;
    case 5: for (int i=0; i<20; i++) { cli(); powerDevice5(); powerOFF();  sei(); delay(7); } delay(1000); break;
  }
}

/////////////////////////// POWER private methods //////////////////////////
void PowerTX434::powerShort0() {
  digitalWriteFast(TX434pin, HIGH);
  delayMicroseconds(250); // 220 on air
  digitalWriteFast(TX434pin, LOW);
  delayMicroseconds(650); // 690 on air
}
void PowerTX434::powerLong1() {
  digitalWriteFast(TX434pin, HIGH);
  delayMicroseconds(700); // 680 on air
  digitalWriteFast(TX434pin, LOW);
  delayMicroseconds(200); // 240 on air
}
void PowerTX434::powerON() {
  for (int i=0; i<8; i++) { powerShort0(); }
  for (int i=0; i<2; i++) { powerLong1(); }
  for (int i=0; i<7; i++) { powerShort0(); }
}
void PowerTX434::powerOFF() {
  for (int i=0; i<17; i++) { powerShort0(); }
}
void PowerTX434::powerDevice1() {
  for (int i=0; i<2; i++) { powerShort0(); }
  for (int i=0; i<6; i++) { powerLong1(); }
}
void PowerTX434::powerDevice2() {
  for (int i=0; i<4; i++) { powerShort0(); }
  for (int i=0; i<4; i++) { powerLong1(); }
}
void PowerTX434::powerDevice3() {
  for (int i=0; i<2; i++) { powerShort0(); }
  for (int i=0; i<2; i++) { powerLong1(); }
  for (int i=0; i<2; i++) { powerShort0(); }
  for (int i=0; i<2; i++) { powerLong1(); }
}
void PowerTX434::powerDevice4() {
  for (int i=0; i<6; i++) { powerShort0(); }
  for (int i=0; i<2; i++) { powerLong1(); }
}
void PowerTX434::powerDevice5() {
  for (int i=0; i<2; i++) { powerShort0(); }
  for (int i=0; i<4; i++) { powerLong1(); }
  for (int i=0; i<2; i++) { powerShort0(); }
}

