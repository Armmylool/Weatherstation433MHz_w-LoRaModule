## 🛰️ 433MHz Weather Station to LoRaWAN/ChirpStack Bridge

This project is designed to capture data from a **433 MHz wireless weather station** using a **RAK811** module. The RAK811 acts as a bridge, receiving the sensor data and relaying it over LoRaWAN to a **ChirpStack** network server. The data is then stored in **InfluxDB** and visualized on a **Grafana** dashboard.

-----

## 🏗️ Project Architecture

This diagram shows the flow of data from the sensor to the final dashboard.

```
[Weather Station] --(433 MHz)--> [RAK811] --(LoRaWAN)--> [ChirpStack] --(Integration)--> [InfluxDB] --> [Grafana]
```

-----

## ✨ Features

  * 📡 **Receives Weather Data**: Captures and decodes data packets from a 433 MHz wireless weather station.
  * 🛰️ **LoRaWAN Transmission**: Forwards the weather data to a private or public LoRaWAN network.
  * ⚙️ **ChirpStack Ready**: Designed to integrate with the ChirpStack open-source LoRaWAN Network Server stack.
  * 🛡️ **OTAA Authentication**: Uses Over-the-Air Activation (OTAA) for secure network joins.
  * 💻 **Serial Debugging**: Provides real-time status updates for easy monitoring and troubleshooting.

-----

## 🛠️ Hardware Requirements

  * **Wireless Weather Station**: Any model that transmits data over the 433 MHz frequency.
  * **RAK811 LoRa Module**: The main microcontroller and LoRa radio.
  * **433 MHz RF Receiver Module**: A standard receiver like the `XY-MK-5V` or a similar model.

-----

## 🔌 Setup and Configuration

### 1\. Hardware Wiring

Connect the 433 MHz receiver module to your RAK811 board.

| Receiver Pin | RAK811 Pin     | Description          |
| :----------- | :------------- | :------------------- |
| **DATA** | `PB12`         | The signal output pin. |
| **VCC** | `3.3V` or `5V` | Power supply.        |
| **GND** | `GND`          | Common Ground.       |

> **Note:** The LoRa-specific pins (`NSS`, `RST`, `DIOs`) are already configured in the code for the RAK811.

### 2\. LoRaWAN Setup (ChirpStack)

You must register your RAK811 as a device in your ChirpStack instance.

1.  Log in to your **ChirpStack** web interface.
2.  Create a **Device profile** for your RAK811 (specifying LoRaWAN version, etc.).
3.  Create an **Application**.
4.  Inside the Application, add a new device. You will need to provide a `Device EUI`.
5.  After creating the device, ChirpStack will provide you with an `AppKey`. The `AppEUI` (or `JoinEUI`) is typically determined by your Application settings.
6.  Take note of the **`AppEUI`**, **`DevEUI`**, and **`AppKey`** for the next step.

### 3\. Code Configuration

Open the main sketch file and update the LoRaWAN credentials to match those from your ChirpStack setup.

**IMPORTANT**: Pay close attention to the **endianness** required by the code.

  * `APPEUI` and `DEVEUI` must be in **Little-Endian** format (reverse the byte order).
  * `APPKEY` must be in **Big-Endian** format (copy as-is).

<!-- end list -->

```cpp
// Update this with the AppEUI/JoinEUI from your ChirpStack Application (in little-endian)
static const u1_t PROGMEM APPEUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// Update this with your device's DevEUI (in little-endian)
static const u1_t PROGMEM DEVEUI[8] = { 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// Update this with the AppKey provided by ChirpStack for your device (in big-endian)
static const u1_t PROGMEM APPKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03 };
```

### 4\. Backend Integration

1.  In **ChirpStack**, set up an integration to push data to an MQTT broker or directly to **InfluxDB** (using the InfluxDB integration).
2.  In **Grafana**, add InfluxDB as a data source.
3.  Create a new dashboard in Grafana and build panels by querying the weather station data from InfluxDB.

----
<img width="1831" height="595" alt="image" src="https://github.com/user-attachments/assets/bf668f20-560f-49da-a8ac-36e1abd1f384" />
