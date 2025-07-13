
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "2_Signal.h"
#include "4_Display.h"
#include "5_Plugin.h"
#include "7_Utils.h"

#define NOT_A_PIN -1
#ifdef COMPILE_REGRESSION_TEST
#define FILLMEIN 0
#else
#warning "You must replace the values marked FILLMEIN with real values from the TTN control panel!"
#define FILLMEIN (#dont edit this, edit the lines that use FILLMEIN)
#endif

#define PIN_RF_RX_VCC NOT_A_PIN  // Power to the receiver on this pin
#define PIN_RF_RX_NA NOT_A_PIN   // Alt. RX_DATA. Forced as input
#define PIN_RF_RX_DATA   PB12       // On this input, the 433Mhz-RF signal is received. LOW when no signal.
#define PIN_RF_RX_GND NOT_A_PIN  // Ground to the receiver on this pin
#define PIN_RF_TX_VCC NOT_A_PIN  // +5 volt / Vcc power to the transmitter on this pin
#define PIN_RF_TX_GND NOT_A_PIN  // Ground power to the transmitter on this pin
#define PIN_RF_TX_DATA NOT_A_PIN // Data to the 433Mhz transmitter on this pin

char data_send[20];

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getArtEui(u1_t* buf) {
  memcpy_P(buf, APPEUI, 8);
}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8] = { 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getDevEui(u1_t* buf) {
  memcpy_P(buf, DEVEUI, 8);
}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
static const u1_t PROGMEM APPKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03 };
void os_getDevKey(u1_t* buf) {
  memcpy_P(buf, APPKEY, 16);
}

static uint8_t lora_data[20];
uint32_t lora_count = 0;
static osjob_t sendjob;

typedef enum {
  LORA_UNCONFIRMED,
  LORA_CONFIRMED
};
// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 5;

// Pin mapping
const lmic_pinmap lmic_pins = {
  .nss = 26,
  .rxtx = 32,
  .rst = 21,
  .dio = { 27, 28, 29 },
};

int freeMemory() {
    register char * stack_ptr asm("sp");
    extern char _end;  // from linker
    return stack_ptr - &_end;
}

void printFreeRAM() {
  Serial.print("Free RAM: ");
  Serial.println(freeMemory());
}



void enableRX()
{
  // RX pins
  pinMode(PIN_RF_RX_DATA, INPUT);        // Initialise in/output ports
  pinMode(PIN_RF_RX_DATA, INPUT_PULLUP); // Initialise in/output ports
}

void disableRX()
{
  // RX pins
  pinMode(PIN_RF_RX_DATA, INPUT);
}

void onEvent(ev_t ev) {
  //Serial.print(os_getTime());
  //Serial.print(": ");
  switch (ev) {
    case EV_SCAN_TIMEOUT:
      Serial.println(F("EV_SCAN_TIMEOUT"));
      break;
    case EV_BEACON_FOUND:
      Serial.println(F("EV_BEACON_FOUND"));
      break;
    case EV_BEACON_MISSED:
      Serial.println(F("EV_BEACON_MISSED"));
      break;
    case EV_BEACON_TRACKED:
      Serial.println(F("EV_BEACON_TRACKED"));
      break;
    case EV_JOINING:
      Serial.println(F("EV_JOINING"));
      break;
    case EV_JOINED:
      Serial.println(F("EV_JOINED"));
      {
        u4_t netid = 0;
        devaddr_t devaddr = 0;
        u1_t nwkKey[16];
        u1_t artKey[16];
        LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
        Serial.print("netid: ");
        Serial.println(netid, DEC);
        Serial.print("devaddr: ");
        Serial.println(devaddr, HEX);
        Serial.print("artKey: ");
        for (size_t i = 0; i < sizeof(artKey); ++i) {
          Serial.print(artKey[i], HEX);
        }
        Serial.println("");
        Serial.print("nwkKey: ");
        for (size_t i = 0; i < sizeof(nwkKey); ++i) {
          Serial.print(nwkKey[i], HEX);
        }
        Serial.println("");

        lora_count = 1;
      }
      // Disable link check validation (automatically enabled
      // during join, but because slow data rates change max TX
      // size, we don't use it in this example.
      LMIC_setLinkCheckMode(0);
      break;
    /*
            || This event is defined but not used in the code. No
            || point in wasting codespace on it.
            ||
            || case EV_RFU1:
            ||     Serial.println(F("EV_RFU1"));
            ||     break;
            */
    case EV_JOIN_FAILED:
      Serial.println(F("EV_JOIN_FAILED"));
      break;
    case EV_REJOIN_FAILED:
      Serial.println(F("EV_REJOIN_FAILED"));
      break;
    case EV_TXCOMPLETE:
      Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
      if (LMIC.txrxFlags & TXRX_ACK)
        Serial.println(F("Received ack\r\n"));
      if (LMIC.dataLen) {
        Serial.print(F("Received "));
        Serial.print(LMIC.dataLen);
        Serial.println(F(" bytes of payload"));
      }
      // Schedule next transmission
      os_setTimedCallback(&sendjob, sec2osticks(TX_INTERVAL), do_send);
      break;
    case EV_LOST_TSYNC:
      Serial.println(F("EV_LOST_TSYNC"));
      break;
    case EV_RESET:
      Serial.println(F("EV_RESET"));
      break;
    case EV_RXCOMPLETE:
      // data received in ping slot
      Serial.println(F("EV_RXCOMPLETE"));
      break;
    case EV_LINK_DEAD:
      Serial.println(F("EV_LINK_DEAD"));
      break;
    case EV_LINK_ALIVE:
      Serial.println(F("EV_LINK_ALIVE"));
      break;
      /*
            || This event is defined but not used in the code. No
            || point in wasting codespace on it.
            ||
            || case EV_SCAN_FOUND:
            ||    Serial.println(F("EV_SCAN_FOUND"));
            ||    break;
            */
      case EV_TXSTART:
      Serial.println(F("EV_TXSTART"));
      break;
    default:
      Serial.print(F("Unknown event: "));
      Serial.println((unsigned)ev);
      break;
  }
}

void sendMsg()
{
  if (pbuffer[0] != 0)
  {
    Serial.print(pbuffer) ;
    Serial.print("ID : "); Serial.println(buffer[6]);
    Serial.print("Temperature : "); Serial.println(buffer[0]);
    Serial.print("Humidity : "); Serial.println(buffer[1]);
    Serial.print("Windspeed : "); Serial.println(buffer[2]);
    Serial.print("Windgust : "); Serial.println(buffer[3]);
    Serial.print("Rain : "); Serial.println(buffer[4]);
    Serial.print("Winddirection : "); Serial.println(buffer[5]);
    
  
 
    sprintf((char*)lora_data, "%de%de%de%de%de%de%de",
        buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5],freeMemory());

    Serial.print("Data Send : ");
    Serial.println((char*)lora_data);

  }
}

void do_send(osjob_t* j) {
    while (true) {
        if (ScanEvent()) {
        Serial.println("ScanEvent");
        sendMsg();
        delay(50) ;
        if (pbuffer[0] != 0){
          pbuffer[0] = 0;
          break ;
        }
    }
  }

    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
      lora_count ++ ;
      if (lora_count == 258) {
        NVIC_SystemReset();
      }
      // Serial.println(lora_data);
        LMIC_setTxData2(1, lora_data, strlen((char*)lora_data), LORA_UNCONFIRMED);
        Serial.println(F("Packet queued"));
        printFreeRAM() ;
    }
}

void setup() {
  Serial.begin(57600);
  while (!Serial) delay(10);
  Serial.begin(BAUD); // Initialise the serial port   
  Serial.println();   // ESP "Garbage" message

  // disableTX();
  enableRX();

  pbuffer[0] = 0;

  PluginInit();
  Serial.println("\r\n*******************************");
  Serial.println("System Starting...");
  Serial.println("");

  pinMode(RADIO_XTAL_EN, OUTPUT);
  digitalWrite(RADIO_XTAL_EN, 1);


  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();

  LMIC_setAdrMode(false);        // ADR
  LMIC_setDrTxpow(DR_SF12, 14);  // DR TxPower

  // Start job (sending automatically starts OTAA too)
  do_send(&sendjob);
}

void loop() {
  os_runloop_once();
}