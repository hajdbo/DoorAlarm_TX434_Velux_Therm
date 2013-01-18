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
ajouter des fils d'antenne pour RX/TX 434
ajouter une horloge pour programmation (ou NTP?)
ajouter une transmission de températeure externe et interne pour aider à réguler le chauffage
*/

// #define DEBUG_OTAX 1
#include  <digitalWriteFast.h>
#include "PowerTX434.h"
#include "Otax.h"
#include "RegulationTemp.h"

#define DHT22_NO_FLOAT
#include <DHT22.h>
//#include <JeeLib.h>   // pour le capteur de temperature
#include "pins.h"

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

Otax myOtax;

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
        myOtax.data = 0;
        myOtax.bits = 0;
        myOtax.state = OTAX_UNKNOWN;
      }
    } else { // LOW
      m=micros();
      lowstart = m;
      highwidth = m - highstart;
    }
    
    if ((myOtax.state != OTAX_DONE) && (val==LOW)) // 800:690-910 1450:1350-1570
        switch ((highwidth + 30) / 220) {
            case 3:  myOtax.state = myOtax.OTAX_bit(0); break;
            case 6:  myOtax.state = myOtax.OTAX_bit(1); break;
            default: myOtax.state = OTAX_UNKNOWN; myOtax.data = 0; myOtax.bits = 0;
        }
}

void setup() {
  pinMode(doorPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(TX434pin, OUTPUT);
  pinMode(RX434pin, INPUT); //pin Digital3 (hardware interrupt 1)
  pinMode(veluxDownPin, OUTPUT);
  pinMode(veluxStopPin, OUTPUT);
  pinMode(veluxUpPin, OUTPUT);
  Serial.begin(57600);
  Serial.println("\ndooralarm starting");
  myOtax.profile = OTAX_PROXY;
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
  if (myOtax.state == OTAX_DONE) {
      myOtax.lastRXtime = millis();
      myOtax.decideWhatToDo();
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
      case 'N': Serial.println("force th off");   myOtax.force=OTAX_FORCEOFF;  break; // OTAX force off
      case 'O': Serial.println("force th on");    myOtax.force=OTAX_FORCEON;   break; // OTAX force on
      case 'P': Serial.println("don't force th"); myOtax.force=OTAX_DONTFORCE; break; // OTAX proxy no-override
      case 'Q': Serial.println("th on");  myOtax.sendOn();  break; // test 1 paquet
      case 'R': Serial.println("th off"); myOtax.sendOff(); break; // test 1 paquet
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
