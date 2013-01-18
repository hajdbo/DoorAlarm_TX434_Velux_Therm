/*
uploadé le 19/11/2012
 signal dooralarm sur digitalpin5, rx434 digitalpin3, tx434 digitalpin7
 10kohm entre pin5 et GND
 10kohm entre pin4 et +5V
 un capteur de temperature sur PIN4

TODO:
timer pour lire la temp/humid chaque x minutes
ordre de temperature idéale
tester la qualité de la réception 434 dans l'alcove
ajouter un fil d'antenne pour RX 434
ajouter une horloge pour programmation (+NTP?)
ajouter une transmission de températeure externe et interne pour aider à réguler le chauffage

////RX 434MHz digital pin3
Decoder for 433 MHz OTAX thermostat
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

// #define DEBUG_OTAX 1
#include  <digitalWriteFast.h>
#include "PowerTX434.h"
#include "Otax.h"
#include "RegulationTemp.h"

#define DHT22_NO_FLOAT
#include <DHT22.h>
//#include <JeeLib.h>   // pour le capteur de temperature
#define TXpin 7
#define RXpin 3
#define doorPin   5
#define ledPin   13
#define DHT22_PIN  4
#define veluxDownPin 10
#define veluxStopPin 11
#define veluxUpPin   12

int DoorState = LOW;       // current state of the button
int lastDoorStateEDGE = LOW;   // previous state of the button (edge detection)
int lastDoorStateDB = LOW;   // previous state of the button (v short, debounce)
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 250;   // <250ms : probablement du bruit sur le fil (allumage de l'entrée par ex)
int temp_demandee = 205; // 20.5° par défaut
int temp_appart_TEMPerUSB = 205;
int temp_appart_DHT22 = 205;
int temp_paris = 116; // moyenne annuelle 11.6°C
byte humid_paris = 78; // moyenne annuelle 78%
byte humid_DHT22 = 65;

enum { UNKNOWN, OK, DONE };
enum { OTAX_PROXY, OTAX_NOPROXY };
enum { OTAX_NOTRECEIVED, OTAX_RXOFF, OTAX_RXON };
enum { OTAX_DONTFORCE, OTAX_FORCEOFF, OTAX_FORCEON, OTAX_REGULATE };

struct { uint8_t state; char bits; uint16_t data; } Otax;

uint8_t OTAXprofile = OTAX_PROXY;
uint8_t OTAXrx = OTAX_NOTRECEIVED;
uint8_t OTAXforce = OTAX_DONTFORCE;
unsigned long OTAXlastTX = 0; // dernier TX vers l'OTAX (ms)
unsigned long OTAXlastRX = 0; // dernier RX de la télécommande OTAX (ms)

uint8_t OTAX_bit(uint8_t value) {
    Otax.data = (Otax.data << 1) | value;
    return ++Otax.bits != 13 ? OK : DONE;
}


// CAPTEUR DE TEMPERATURE/HUMIDITE DHT22
// deux libs au choix : jeelib ou dht22
// #include <DHT22.h>
// https://github.com/ringerc/Arduino-DHT22
//////jeelib:
//////DHTxx dht22(4); // temperature sur le pin 4
DHT22 myDHT22(DHT22_PIN);
int dht22_temp, dht22_humid;


//////////////////////// OTAX //////////////////
void RX434interrupt() {
    // width is the positive pulse length in usecs
    static uint16_t highstart;
    static uint16_t lowstart;
    uint16_t m;
    uint16_t highwidth = 0;
    uint16_t lowwidth = 0;
    
    int val = digitalReadFast(3);
    if (val == HIGH) {
      m=micros();
      highstart = m;
      lowwidth = m - lowstart;
      if (lowwidth > 3000) {
        Otax.data = 0;
        Otax.bits = 0;
        Otax.state = UNKNOWN;
      }
    } else { // LOW
      m=micros();
      lowstart = m;
      highwidth = m - highstart;
    }
    
    if ((Otax.state != DONE) && (val==LOW)) // 800:690-910 1450:1350-1570
        switch ((highwidth + 30) / 220) {
            case 3:  Otax.state = OTAX_bit(0); break;
            case 6:  Otax.state = OTAX_bit(1); break;
            default: Otax.state = UNKNOWN; Otax.data = 0; Otax.bits = 0;
        }
}

void setup() {
  pinMode(doorPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(TXpin, OUTPUT);
  pinMode(RXpin, INPUT); //pin Digital3 (hardware interrupt 1)
  pinMode(veluxDownPin, OUTPUT);
  pinMode(veluxStopPin, OUTPUT);
  pinMode(veluxUpPin, OUTPUT);
  Serial.begin(57600);
  Serial.println("\ndooralarm starting");
  OTAXprofile = OTAX_PROXY;
  attachInterrupt(1, RX434interrupt, CHANGE);
}

void loop() {
  ///////////////////// DOORALARM ///////////////////////////////////
  int reading = digitalRead(doorPin);
  if (reading != lastDoorStateDB)
    lastDebounceTime = millis(); 
  if ((millis() - lastDebounceTime) > debounceDelay)
    DoorState = reading;
  if (DoorState != lastDoorStateEDGE) {
    if (DoorState == HIGH)
      Serial.println("door open");
    else
      Serial.println("door close");
    lastDoorStateEDGE = DoorState;
  } 
  lastDoorStateDB = reading;
  
  //////////////////// Thermostat OTAX RX 434 ////////////////////////////////////
  if (Otax.state == DONE) {
      OTAXlastRX = millis();
      
#ifdef DEBUG_OTAX
      Serial.print("OTAX RX ");
      Serial.println(Otax.data, HEX);
#endif 
      
      Otax.state = UNKNOWN;
      switch (Otax.data) {
        case 0xDB2: // remote manual ON
        case 0xDB4: // remote manual OFF
        case 0xDB6: // remote manual end-of-msg
                    OTAXprofile=OTAX_NOPROXY; // on a passé la télécommande en mode manuel, on ne fait plus rien
                    break;                    // on s'arrete de transmettre (relayer/modifier) à la chaudière

        case 0xCB2: OTAXprofile=OTAX_PROXY; OTAXrx=OTAX_RXON;  break; // remote PC ON
        case 0xCB4: OTAXprofile=OTAX_PROXY; OTAXrx=OTAX_RXOFF; break; // remote PC OFF
        case 0xCB6: OTAXprofile=OTAX_PROXY;                  break; // remote PC end-of-msg
                     // si on recoit CB2,CB4,CB6: qq'un a demandé le mode "PC"
      }
  }
  
  if ((OTAXprofile == OTAX_PROXY) && (OTAXlastTX + 60000 < millis()) && (OTAXlastRX + 10000 < millis())) {
      if (( (OTAXrx == OTAX_RXON)&&(OTAXforce==OTAX_DONTFORCE)) || (OTAXforce==OTAX_FORCEON)) {
        Serial.println("TXch ON");
        OTAXon();
      } else if (( (OTAXrx == OTAX_RXOFF)&&(OTAXforce==OTAX_DONTFORCE)) || (OTAXforce==OTAX_FORCEOFF)) {
        Serial.println("TXch OFF");
        OTAXoff();
      }
      OTAXlastTX = millis();
  }
  
  //////////////////// SERIAL CMD ////////////////////////////////////
  byte cmd; 
  if (Serial.available()) {
    cmd = Serial.read();
    switch (cmd) {
      case 'A': PowerTX434::powerOn(1); break;
      case 'B': PowerTX434::powerOff(1); break;
      case 'C': PowerTX434::powerOn(2); break;
      case 'D': PowerTX434::powerOff(2); break;
      case 'E': PowerTX434::powerOn(3); break;
      case 'F': PowerTX434::powerOff(3); break;
      case 'G': PowerTX434::powerOn(4); break;
      case 'H': PowerTX434::powerOff(4); break;
      case 'I': PowerTX434::powerOn(5); break;
      case 'J': PowerTX434::powerOff(5); break;
      case 'K': digitalWrite(veluxDownPin, HIGH); delay(200); digitalWrite(veluxDownPin, LOW); break;
      case 'L': digitalWrite(veluxStopPin, HIGH); delay(200); digitalWrite(veluxStopPin, LOW); break;
      case 'M': digitalWrite(veluxUpPin, HIGH);   delay(200); digitalWrite(veluxUpPin, LOW);   break;
      case 'N': Serial.println("force th off");   OTAXforce=OTAX_FORCEOFF;  break; // OTAX force off
      case 'O': Serial.println("force th on");    OTAXforce=OTAX_FORCEON;   break; // OTAX force on
      case 'P': Serial.println("don't force th"); OTAXforce=OTAX_DONTFORCE; break; // OTAX proxy no-override
      case 'Q': Serial.println("th on");  OTAXon();  break; // test 1 paquet
      case 'R': Serial.println("th off"); OTAXoff(); break; // test 1 paquet
      case 'S': {// data externe temp_appart/temp_paris/%humid_paris   S205;-013;056
        while (Serial.available() < 12)  { delay(100); }
        temp_appart_TEMPerUSB  = (Serial.read()-48)*100; // ascii '0'=48(dec)
        temp_appart_TEMPerUSB += (Serial.read()-48)*10;
        temp_appart_TEMPerUSB +=  Serial.read()-48;
        Serial.read(); // c'est le separateur ";"
        char signe = Serial.read();
        temp_paris  = (Serial.read()-48)*100; // ascii '0'=48(dec)
        temp_paris += (Serial.read()-48)*10;
        temp_paris +=  Serial.read()-48;
        if (signe == '-') { temp_paris = 0 - temp_paris; } // - ou 0
        Serial.read(); // c'est le separateur ";"
        humid_paris  = (Serial.read()-48)*100; // ascii '0'=48(dec)
        humid_paris += (Serial.read()-48)*10;
        humid_paris +=  Serial.read()-48;
      } break;
      case 'T': {  // 3 ascii temperature demandee  ex: T195 pour 19.5°C,  T205 pour 20.5°C
        while (Serial.available() < 3)  { delay(100); }
        temp_demandee  = (Serial.read()-48)*100; // ascii '0'=48(dec)
        temp_demandee += (Serial.read()-48)*10;
        temp_demandee +=  Serial.read()-48;
        } break; 
      /*case 'U': dht22.reading(dht22_temp,dht22_humid);// valeur T*10
                Serial.print("T:"); Serial.println(dht22_temp); 
                Serial.print("H:"); Serial.println(dht22_humid);
                break;*/
      case 'U': {   
                  DHT22_ERROR_t errorCode;
                  detachInterrupt(1);
                  errorCode = myDHT22.readData();
                  attachInterrupt(1, RX434interrupt, CHANGE);
                  switch(errorCode)
                  {
                    case DHT_ERROR_NONE:
                              Serial.print(myDHT22.getTemperatureCInt());
                              Serial.print("C ");
                              Serial.print(myDHT22.getHumidityInt());
                              Serial.println("%");
                              break;
                  }
                }   
                break;

      default: break;
    }
  } 
}
