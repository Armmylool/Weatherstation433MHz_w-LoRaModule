#include <Arduino.h>
#include "2_Signal.h"
#include "4_Display.h"
#include "5_Plugin.h"
void sendMsg(); 

#define PIN_RF_RX_VCC NOT_A_PIN  // Power to the receiver on this pin
#define PIN_RF_RX_NA NOT_A_PIN   // Alt. RX_DATA. Forced as input
#define PIN_RF_RX_DATA 13         // On this input, the 433Mhz-RF signal is received. LOW when no signal.
#define PIN_RF_RX_GND NOT_A_PIN  // Ground to the receiver on this pin
#define PIN_RF_TX_VCC NOT_A_PIN  // +5 volt / Vcc power to the transmitter on this pin
#define PIN_RF_TX_GND NOT_A_PIN  // Ground power to the transmitter on this pin
#define PIN_RF_TX_DATA NOT_A_PIN // Data to the 433Mhz transmitter on this pin


void setup()
{
  delay(150);         // Time needed to switch back from Upload to Console
  Serial.begin(BAUD); // Initialise the serial port
  Serial.println();   // ESP "Garbage" message

  // disableTX();
  enableRX();

  pbuffer[0] = 0;

  PluginInit();
  delay(100);
}

void loop()
{
  if (ScanEvent())
    sendMsg();
}

void sendMsg()
{
  if (pbuffer[0] != 0)
  {
    Serial.print(pbuffer) ;
    Serial.print("ID : ") ; Serial.println(buffer[6]) ;
    Serial.print("Temperature : ") ; Serial.println(buffer[0]) ;
    Serial.print("Humidity : ") ; Serial.println(buffer[1]) ;
    Serial.print("Windspeed : ") ; Serial.println(buffer[2]) ;
    Serial.print("Windgust : ") ; Serial.println(buffer[3]) ;
    Serial.print("Rain : ") ; Serial.println(buffer[4]) ;
    Serial.print("Winddirection : ") ; Serial.println(buffer[5]) ;



    pbuffer[0] = 0;
  }
}

void enableRX()
{
  // RX pins
  pinMode(PIN_RF_RX_GND, OUTPUT);        // Initialise in/output ports
  pinMode(PIN_RF_RX_VCC, OUTPUT);        // Initialise in/output ports
  pinMode(PIN_RF_RX_NA, INPUT);          // Initialise in/output ports
  pinMode(PIN_RF_RX_DATA, INPUT);        // Initialise in/output ports
  digitalWrite(PIN_RF_RX_GND, LOW);      // turn GND to RF receiver ON
  digitalWrite(PIN_RF_RX_VCC, HIGH);     // turn VCC to RF receiver ON
  pinMode(PIN_RF_RX_DATA, INPUT_PULLUP); // Initialise in/output ports
}

void disableRX()
{
  // RX pins
  pinMode(PIN_RF_RX_DATA, INPUT);
  pinMode(PIN_RF_RX_NA, INPUT);
  pinMode(PIN_RF_RX_VCC, INPUT);
  pinMode(PIN_RF_RX_GND, INPUT);
}