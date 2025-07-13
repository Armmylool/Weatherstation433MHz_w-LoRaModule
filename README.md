🛰️ RAK811: 433MHz to LoRaWAN Bridge
This project acts as a bridge, capturing data from a 433 MHz wireless sensor (like a weather station) using a RAK811 module. It then relays this data to a LoRaWAN network, such as The Things Network (TTN).

✨ Features
Receives 433 MHz Signals: Captures and processes data packets from common 433 MHz RF modules.

LoRaWAN Transmission: Formats the captured data and transmits it over a LoRaWAN network using the MCCI LMIC library.

OTAA Authentication: Utilizes Over-the-Air Activation (OTAA) for securely joining the LoRaWAN network.

Serial Debugging: Provides clear status updates, received data, and transmission events through the Serial Monitor for easy debugging.

RAK811 Optimized: The code is specifically configured for the RAK811's pin mapping.

🛠️ Hardware Requirements
RAK811 LoRa Module: The main microcontroller and LoRa radio.

433 MHz RF Receiver Module: A standard receiver like the XY-MK-5V or a similar model.

(Optional) 433 MHz Transmitter: The source of the signal, such as a wireless weather station or an Arduino with a 433 MHz transmitter module.

⚙️ Setup and Configuration
1. Hardware Connection
Connect the 433 MHz receiver module to your RAK811 board as follows:

DATA Pin of the receiver ➡️ Pin PB12 on the RAK811

VCC Pin of the receiver ➡️ A suitable power source (e.g., 3.3V or 5V, depending on your module)

GND Pin of the receiver ➡️ GND on the RAK811

Note: The LoRa-specific pins (NSS, RST, DIOs) are already defined in the code for the RAK811 and do not need to be changed.

2. LoRaWAN Setup (The Things Network)
You must first register your device on a LoRaWAN network.

Log in to your account on The Things Network.

Create a new Application.

Inside your application, navigate to End devices and click Add end device.

Choose the Enter end device specifics manually registration method.

Select the correct Frequency plan for your region (e.g., EU868, US915, AS923_4).

Set the LoRaWAN version to LoRaWAN Specification 1.0.2 or higher.

Click Generate to create a DevEUI and AppKey. The AppEUI will be assigned automatically.

Copy these three values: AppEUI, DevEUI, and AppKey.

3. Code Configuration
Open the main .ino sketch file and update the APPEUI, DEVEUI, and APPKEY arrays with the values you obtained from TTN.

IMPORTANT: Pay close attention to the endianness comments in the code.

APPEUI and DEVEUI must be in Little-Endian format (reverse the byte order shown on TTN).

APPKEY must be in Big-Endian format (copy it directly as shown on TTN).

C++

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes.
static const u1_t PROGMEM APPEUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // <-- EDIT THIS

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8] = { 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // <-- EDIT THIS

// This key should be in big endian format. A key from ttnctl can be copied as-is.
static const u1_t PROGMEM APPKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03 }; // <-- EDIT THIS
4. Compile and Upload
Ensure you have the MCCI LoRaWAN LMIC library installed in your Arduino IDE.

Make sure all other project header files (2_Signal.h, 4_Display.h, etc.) are in the same directory as your main sketch.

Select RAK811 as your board in the Arduino IDE.

Compile and upload the code.

▶️ How It Works
setup(): Initializes the Serial port, enables the PIN_RF_RX_DATA input to listen for 433 MHz signals, and initializes the LMIC LoRaWAN stack.

loop(): Continuously calls os_runloop_once(), which is the core state machine for the LMIC library that manages LoRaWAN jobs (transmissions, receptions, etc.).

do_send(): This function is scheduled to run periodically. It calls ScanEvent() to check for new data from the 433 MHz receiver. If data is found, it proceeds to call sendMsg().

sendMsg(): This function processes the raw data received from the sensor (e.g., temperature, humidity) and formats it into a single string payload.

LMIC_setTxData2(): The formatted payload is queued for transmission over LoRaWAN.

onEvent(): This callback function handles all events from the LMIC stack, such as EV_JOINED (successfully joined the network) and EV_TXCOMPLETE (transmission finished), printing status messages to the Serial Monitor.
