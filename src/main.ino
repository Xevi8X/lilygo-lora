#include <Arduino.h>
#include <Wire.h>               
#include "SSD1306Wire.h"
#include <SPI.h>
#include "utilities.h"
#include <LoRa.h>

SSD1306Wire display(0x3c, I2C_SDA, I2C_SCL);
uint32_t last_transmition = 0;
constexpr uint32_t interval = 200;

struct
{
    bool ready;
    String message;
    int RSSI;
    float SNR;
} last_message;

int counter = 0;

void setup()
{
    pinMode(BOARD_LED, OUTPUT);

    Serial.begin(9600);
    Serial.println("Initialized!");
    SPI.begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN);
    Wire.begin(I2C_SDA, I2C_SCL);

    // When the power is turned on, a delay is required.
    delay(1500);

    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);

    

    LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DIO0_PIN);
    if (!LoRa.begin(LoRa_frequency)) {
        Serial.println("Starting LoRa failed!");
        while (1);
    }

    LoRa.onReceive(onReceive);
    LoRa.receive();

    Serial.println("LoRa Receiver");

    
}

void loop()
{   
    // Display incomming messages
    noInterrupts();
    if(last_message.ready)
    {
        // display.clear();
        // display.drawString(0, 12, last_message.message.c_str());
        // char buf[256];
        // snprintf(buf, sizeof(buf), "RSSI:%i", last_message.RSSI);
        // display.drawString(0, 24, buf);
        // snprintf(buf, sizeof(buf), "SNR:%.1f", last_message.SNR);
        // display.drawString(0, 40, buf);
        // display.display();

        Serial.println("======================");
        Serial.println(last_message.message);
        char buf[256];
        snprintf(buf, sizeof(buf), "RSSI:%i", last_message.RSSI);
        Serial.println(buf);
        snprintf(buf, sizeof(buf), "SNR:%.1f", last_message.SNR);
        Serial.println(buf);
        last_message.ready = false;
    }
    interrupts();

    if (millis() - last_transmition > interval) 
    {
        send_message();
        last_transmition = millis();
    }
}

void onReceive(int packetSize) {
    if (packetSize == 0) return;

    String recv = "";
    // read packet
    while (LoRa.available()) {
        recv += (char)LoRa.read();
    }

    last_message.message = recv;
    last_message.RSSI = LoRa.packetRssi();
    last_message.SNR = LoRa.packetSnr();
    last_message.ready = true;
}

void send_message() 
{
    if(0 == LoRa.beginPacket())
    {
        return;
    }
    LoRa.print("PIWO KURWAAAA ");
    LoRa.println(counter++);
    LoRa.endPacket();
    LoRa.receive();
}
