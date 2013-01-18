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


void Otax::decideWhatToDo() {
  #ifdef DEBUG_OTAX
      Serial.print("OTAX RX ");
      Serial.println(data, HEX);
  #endif 
      
      state = OTAX_UNKNOWN;
      switch (data) {
        case 0xDB2: // remote set on NORMAL mode sent cmd "ON"
        case 0xDB4: // remote set on NORMAL mode sent cmd "OFF"
        case 0xDB6: // remote manual end-of-msg
                    profile=OTAX_NOPROXY; // on a passé la télécommande en mode manuel, on ne fait plus rien
                    break;                    // on s'arrete de transmettre (relayer/modifier) à la chaudière

        case 0xCB2: profile=OTAX_PROXY; rx=OTAX_RXON;  break; // remote set on PC mode sent cmd "ON"
        case 0xCB4: profile=OTAX_PROXY; rx=OTAX_RXOFF; break; // remote set on PC mode sent cmd "OFF"
        case 0xCB6: profile=OTAX_PROXY;                break; // remote PC end-of-msg
                     // si on recoit CB2,CB4,CB6: la télécommande est en position "PC"
      }
  
  if ((profile == OTAX_PROXY) && (lastTXtime + 60000 < millis()) && (lastRXtime + 10000 < millis())) { 
  // si on a envoyé il y a plus de 60s et reçu depuis plus de 10s
      if (( (rx == OTAX_RXON)&&(force==OTAX_DONTFORCE)) || (force==OTAX_FORCEON)) { // ici on peut ajouter || temp < tempdemandee (ajouter une tolerance à la montée/descente)
        Serial.println("TXch ON");
        sendOn();
      } else if (( (rx == OTAX_RXOFF)&&(force==OTAX_DONTFORCE)) || (force==OTAX_FORCEOFF)) { // ici on peut ajouter || temp > tempdemandee (ajouter une tolerance à la montée/descente)
        Serial.println("TXch OFF");
        sendOff();
      }
      lastTXtime = millis();
  }
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
    return ++bits != 13 ? OTAX_OK : OTAX_DONE;
}

