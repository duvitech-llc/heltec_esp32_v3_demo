/* Heltec LoRa transmitter/receiver example
 * Uses RadioLib which properly supports V3 boards
 */

#include <Wire.h>
#include <SPI.h>
#include "HT_SSD1306Wire.h"
#include <RadioLib.h>

// MODE SELECTION: Set to true for transmitter, false for receiver
#define IS_TRANSMITTER true

// Create display instance
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);

#define RF_FREQUENCY                                915.0    // MHz
#define TX_OUTPUT_POWER                             5        // dBm
#define LORA_BANDWIDTH                              125.0    // kHz
#define LORA_SPREADING_FACTOR                       7        // [SF7..SF12]
#define LORA_CODINGRATE                             5        // [5: 4/5, 6: 4/6, 7: 4/7, 8: 4/8]
#define LORA_PREAMBLE_LENGTH                        8

#define BUFFER_SIZE                                 30

// Pin definitions for Heltec WiFi LoRa 32 V3
#define RADIO_SCLK_PIN               9
#define RADIO_MISO_PIN              11
#define RADIO_MOSI_PIN              10
#define RADIO_CS_PIN                 8
#define RADIO_DIO1_PIN              14
#define RADIO_RST_PIN               12
#define RADIO_BUSY_PIN              13

// Create radio instance (SX1262 for V3 board)
SX1262 radio = new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);

char txpacket[BUFFER_SIZE];
double txNumber = 0;

#if !IS_TRANSMITTER
// Receiver-specific variables
int16_t rssi_value, snr_value;
volatile bool receivedFlag = false;

// This function is called when a complete packet is received
void setFlag(void) {
    receivedFlag = true;
}
#endif

void setup() {
    Serial.begin(115200);
    
    // Turn on Vext power for display
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);  // Vext is active LOW
    delay(100);
    
    // Initialize display BEFORE SPI
    display.init();
    display.setContrast(255);
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    
    // Display initialization message
    display.drawString(0, 0, "LoRa TX");
    display.drawString(0, 20, "Initializing...");
    display.display();
    
    // Initialize SPI for LoRa radio
    SPI.begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN, RADIO_CS_PIN);
    
    Serial.print(F("[SX1262] Initializing ... "));
    
    // Initialize SX1262
    int state = radio.begin();
    
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println(F("success!"));
        
        // Configure the radio
        radio.setFrequency(RF_FREQUENCY);
        radio.setBandwidth(LORA_BANDWIDTH);
        radio.setSpreadingFactor(LORA_SPREADING_FACTOR);
        radio.setCodingRate(LORA_CODINGRATE);
        radio.setOutputPower(TX_OUTPUT_POWER);
        radio.setPreambleLength(LORA_PREAMBLE_LENGTH);
        
        display.clear();
#if IS_TRANSMITTER
        display.drawString(0, 0, "LoRa TX Ready");
#else
        display.drawString(0, 0, "LoRa RX Ready");
#endif
        display.display();
        
    } else {
        Serial.print(F("failed, code "));
        Serial.println(state);
        display.clear();
        display.drawString(0, 0, "Radio FAIL");
        display.drawString(0, 20, "Code: " + String(state));
        display.display();
        while (true) { delay(10); }
    }
    
#if IS_TRANSMITTER
    Serial.println("Transmitter initialized");
#else
    Serial.println("Receiver initialized");
    
    // Set interrupt for received packets
    radio.setDio1Action(setFlag);
    
    // Start listening
    Serial.print(F("[SX1262] Starting to listen ... "));
    state = radio.startReceive();
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println(F("success!"));
    } else {
        Serial.print(F("failed, code "));
        Serial.println(state);
        while (true) { delay(10); }
    }
#endif
}

void loop() {
#if IS_TRANSMITTER
    // Transmitter mode
    delay(1000);
    
    txNumber += 0.01;
    sprintf(txpacket, "Hello world %0.2f", txNumber);
    
    Serial.printf("\r\nsending packet \"%s\", length %d\r\n", txpacket, strlen(txpacket));
    
    // Update display
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "Transmitting:");
    display.drawString(0, 16, txpacket);
    display.display();
    
    // Transmit
    int state = radio.transmit((uint8_t *)txpacket, strlen(txpacket));
    
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println("TX done......");
        display.drawString(0, 32, "TX Success!");
        display.display();
    } else {
        Serial.printf("TX failed, code %d\n", state);
        display.drawString(0, 32, "TX FAIL: " + String(state));
        display.display();
    }
#else
    // Receiver mode
    if(receivedFlag) {
        receivedFlag = false;
        
        String str;
        int state = radio.readData(str);
        
        if (state == RADIOLIB_ERR_NONE) {
            // Packet received successfully
            Serial.println(F("[SX1262] Received packet!"));
            
            rssi_value = radio.getRSSI();
            snr_value = radio.getSNR();
            
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
        
        radio.startReceive();
    }
#endif
}