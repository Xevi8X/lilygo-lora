#include <Arduino.h>
#include <Wire.h>               
#include "SSD1306Wire.h"
#include <SPI.h>
#include "utilities.h"
#include <LoRa.h>
#include "images.h"
#include <CRC16.h>

CRC16 crc16;

SSD1306Wire display(0x3c, I2C_SDA, I2C_SCL);
uint32_t last_transmition = 0;
constexpr uint32_t interval = 200;
constexpr uint8_t header[4] = {'P', 'I', 'W', 'O'};

struct Message
{
    uint8_t header[4];
    uint16_t counter;
    uint16_t crc16;
};

struct
{
    bool ready;
    Message message;
    int RSSI;
    float SNR;
} last_message;



int counter = 0;

Message from_bytes(uint8_t bytes){

}

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
    display.setFont(ArialMT_Plain_16);

    
    LoRa.setSignalBandwidth(500E6);
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
    //noInterrupts();
    if(last_message.ready)
    {
        char buf[256];

        // Serial.println("======================");
        // Serial.println(last_message.message);
        // snprintf(buf, sizeof(buf), "RSSI:%i", last_message.RSSI);
        // Serial.println(buf);
        // snprintf(buf, sizeof(buf), "SNR:%.1f", last_message.SNR);
        // Serial.println(buf);

        display.clear();
        display.drawString(0, 0, last_message .c_str());
        snprintf(buf, sizeof(buf), "RSSI:%i", last_message.RSSI);
        display.drawString(0, 18, buf);
        snprintf(buf, sizeof(buf), "SNR:%.1f", last_message.SNR);
        display.drawString(0, 36, buf);
        display.drawXbm(128-beer_width, 64-beer_height,
            beer_width, beer_height, inverted_beer_bits);
        display.display();

        last_message.ready = false;
    }
    //interrupts();

    if (millis() - last_transmition > interval) 
    {
        send_message();
        last_transmition = millis();
    }
}

void onReceive(int packetSize) {
    if (packetSize != sizeof(Message)) return;

    uint8_t buffer[sizeof(Message)];
    LoRa.readBytes(static_cast<uint8_t*>(&buffer),sizeof(Message));

    memcpy(&last_message.message, reinterpret_cast<Message*>(&buffer), sizeof(Message));
    last_message.RSSI = LoRa.packetRssi();
    last_message.SNR = LoRa.packetSnr();
    last_message.ready = true;
}

void send_message() 
{
    if(0 == LoRa.beginPacket())
    {
        LoRa.receive();
        return;
    }
    Message msg;
    memcpy(&msg.header,&header,sizeof(header));
    msg.counter = counter++;
    msg.crc16 = crc16.calculateCRC(reinterpret_cast<uint8_t*>(&msg), offsetof(Message, crc16));
    LoRa.write(reinterpret_cast<uint8_t*>(&msg), sizeof(msg));
    LoRa.endPacket();
    LoRa.receive();
}
