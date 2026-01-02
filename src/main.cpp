// For a connection via I2C using the Arduino Wire include:
#include <Wire.h>
#include <SPI.h>
#include "HT_SSD1306Wire.h"
#include <RadioLib.h>

// MODE SELECTION: Set to true for transmitter, false for receiver
#define IS_TRANSMITTER false

// Create display instance using board-specific pin definitions from pins_arduino.h
// For WiFi LoRa 32 V3: SDA_OLED=17, SCL_OLED=18, RST_OLED=21
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);

#define RF_FREQUENCY                                915.0 // MHz
#define TX_OUTPUT_POWER                             14        // dBm
#define LORA_BANDWIDTH                              125.0     // kHz
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             5         // [5: 4/5, 6: 4/6, 7: 4/7, 8: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx

#define BUFFER_SIZE                                 30 // Define the payload size here

// Pin definitions for Heltec WiFi LoRa 32 V3
#define RADIO_SCLK_PIN               9
#define RADIO_MISO_PIN              11
#define RADIO_MOSI_PIN              10
#define RADIO_CS_PIN                 8
#define RADIO_DIO1_PIN              14
#define RADIO_RST_PIN               12
#define RADIO_BUSY_PIN              13
#define LORA_PWR_PIN                36  // LoRa power enable

// Create radio instance (SX1262 for V3 board)
SX1262 radio = new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);

int counter = 0;
char rxpacket[BUFFER_SIZE];
int16_t rssi_value, snr_value;
bool lora_idle = true;

// Flag to indicate that a packet was received
volatile bool receivedFlag = false;

// This function is called when a complete packet is received by the module
void setFlag(void) {
  receivedFlag = true;
}

void setup() {
  // Initialize Serial
  Serial.begin(115200);
  
  // Turn on Vext power for display
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);  // Vext is active LOW
  delay(100);
  
  // Turn on LoRa radio power
  pinMode(LORA_PWR_PIN, OUTPUT);
  digitalWrite(LORA_PWR_PIN, HIGH);  // Enable LoRa power
  delay(100);
  
  // Initialize SPI for LoRa radio
  SPI.begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN, RADIO_CS_PIN);
  
  // Initialize display
  display.init();
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  
  // Display initialization message
  display.drawString(0, 0, "Heltec WiFi LoRa");
  display.drawString(0, 20, "V3 Display");
  display.drawString(0, 40, "Initializing...");
  display.display();
  
  Serial.print(F("[SX1262] Initializing ... "));
  
  // Initialize SX1262 with default settings first
  int state = radio.begin();
  
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
    
    // Now configure the radio
    state = radio.setFrequency(RF_FREQUENCY);
    if (state != RADIOLIB_ERR_NONE) {
      Serial.print(F("Failed to set frequency, code "));
      Serial.println(state);
    }
    
    state = radio.setBandwidth(LORA_BANDWIDTH);
    if (state != RADIOLIB_ERR_NONE) {
      Serial.print(F("Failed to set bandwidth, code "));
      Serial.println(state);
    }
    
    state = radio.setSpreadingFactor(LORA_SPREADING_FACTOR);
    if (state != RADIOLIB_ERR_NONE) {
      Serial.print(F("Failed to set spreading factor, code "));
      Serial.println(state);
    }
    
    state = radio.setCodingRate(LORA_CODINGRATE);
    if (state != RADIOLIB_ERR_NONE) {
      Serial.print(F("Failed to set coding rate, code "));
      Serial.println(state);
    }
    
    state = radio.setOutputPower(TX_OUTPUT_POWER);
    if (state != RADIOLIB_ERR_NONE) {
      Serial.print(F("Failed to set output power, code "));
      Serial.println(state);
    }
    
    state = radio.setPreambleLength(LORA_PREAMBLE_LENGTH);
    if (state != RADIOLIB_ERR_NONE) {
      Serial.print(F("Failed to set preamble length, code "));
      Serial.println(state);
    }
    
    display.clear();
    display.drawString(0, 0, "Radio Init OK");
    display.display();
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    display.clear();
    display.drawString(0, 0, "Radio Init FAIL");
    display.drawString(0, 20, "Code: " + String(state));
    display.display();
    while (true) { delay(10); }
  }
  
#if IS_TRANSMITTER
  // Transmitter mode
  Serial.println("Mode: TRANSMITTER");
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "Mode: TX");
  display.display();
  delay(1000);  // Show mode briefly
#else
  // Receiver mode
  Serial.println("Mode: RECEIVER");
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "Mode: RX");
  display.drawString(0, 20, "Listening...");
  display.display();
  delay(1000);  // Show mode briefly
  
  // Set the function that will be called when a packet is received
  radio.setDio1Action(setFlag);
  
  // Start listening for LoRa packets
  Serial.print(F("[SX1262] Starting to listen ... "));
  state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }
#endif
  
  Serial.println("ESP32 initialized");
}

void loop() {
#if IS_TRANSMITTER
  // Transmitter mode - send packets periodically
  Serial.print(F("[SX1262] Transmitting packet ... "));
  
  // Prepare packet
  String message = "Hello World! #" + String(counter++);
  
  // Update display
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Transmitting:");
  display.drawString(0, 12, message.substring(0, 20));
  display.drawString(0, 24, "Count: " + String(counter));
  display.display();
  
  // Transmit packet
  int state = radio.transmit(message);
  
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
    Serial.print(F("[SX1262] Data:\t\t"));
    Serial.println(message);
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
  }
  
  // Wait before next transmission
  delay(2000);
  
#else
  // Receiver mode - check for incoming packets
  // Check if the flag is set
  if(receivedFlag) {
    // Reset flag
    receivedFlag = false;
    
    // Read received packet
    String str;
    int state = radio.readData(str);
    
    if (state == RADIOLIB_ERR_NONE) {
      // Packet received successfully
      Serial.println(F("[SX1262] Received packet!"));
      
      // Get RSSI and SNR
      rssi_value = radio.getRSSI();
      snr_value = radio.getSNR();
      
      // Print data
      Serial.print(F("[SX1262] Data:\t\t"));
      Serial.println(str);
      Serial.print(F("[SX1262] RSSI:\t\t"));
      Serial.print(rssi_value);
      Serial.println(F(" dBm"));
      Serial.print(F("[SX1262] SNR:\t\t"));
      Serial.print(snr_value);
      Serial.println(F(" dB"));
      
      // Update display
      display.clear();
      display.setFont(ArialMT_Plain_10);
      display.drawString(0, 0, "Received:");
      display.drawString(0, 12, str.substring(0, 20));
      display.drawString(0, 24, "RSSI: " + String(rssi_value) + " dBm");
      display.drawString(0, 36, "SNR: " + String(snr_value) + " dB");
      display.display();
      
    } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
      Serial.println(F("[SX1262] CRC error!"));
    } else {
      Serial.print(F("[SX1262] Failed, code "));
      Serial.println(state);
    }
    
    // Put module back to listen mode
    radio.startReceive();
  }
#endif
}
