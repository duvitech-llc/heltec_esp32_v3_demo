# ESP32 LoRa Transmitter/Receiver

A simple LoRa communication project for the Heltec WiFi LoRa 32 V3 board using RadioLib.

## Hardware

- **Board**: Heltec WiFi LoRa 32 V3 (ESP32-S3)
- **Radio**: SX1262 LoRa transceiver
- **Display**: SSD1306 OLED (128x64, I2C)

## Features

- Switchable transmitter/receiver modes via compile-time define
- OLED display showing transmission status or received packets
- Serial output for debugging
- Displays RSSI and SNR for received packets

## LoRa Configuration

- **Frequency**: 915 MHz
- **Spreading Factor**: 7
- **Bandwidth**: 125 kHz
- **Coding Rate**: 4/5
- **TX Power**: 5 dBm
- **Preamble Length**: 8 symbols

## Pin Configuration

### Radio (SX1262)
- CS: GPIO 8
- DIO1: GPIO 14
- RST: GPIO 12
- BUSY: GPIO 13
- SCLK: GPIO 9
- MISO: GPIO 11
- MOSI: GPIO 10

### Display (SSD1306)
- SDA: GPIO 17
- SCL: GPIO 18
- RST: GPIO 21

## Usage

### Building and Uploading

```bash
# Build the project
pio run

# Upload to board
pio run -t upload

# Upload and monitor serial output
pio run -t upload -t monitor
```

### Switching Between Transmitter and Receiver

Edit `src/main.cpp` and change the `IS_TRANSMITTER` define:

```cpp
#define IS_TRANSMITTER true   // For transmitter mode
#define IS_TRANSMITTER false  // For receiver mode
```

Then rebuild and upload the code.

### Transmitter Mode

- Sends "Hello world number X.XX" packets every second
- Displays transmission status on OLED
- Shows TX success/failure in serial monitor

### Receiver Mode

- Listens for incoming LoRa packets
- Displays received message, RSSI, and SNR on OLED
- Prints packet details to serial monitor

## Dependencies

- **RadioLib** (v7.4.0): For SX1262 radio control
- **Heltec ESP32 Dev-Boards** (v2.1.5): For OLED display drivers

## Notes

- The Heltec ESP32 Dev-Boards library is only used for display functionality
- RadioLib is used directly for all radio operations due to incomplete ESP32-S3 support in Heltec's LoRaWAN implementation
- Display must be initialized before SPI to prevent I2C bus conflicts

## License

MIT License - see LICENSE file for details
