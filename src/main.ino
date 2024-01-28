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

bool check_header(uint8_t msg_header[4])
{
    return header[0] == msg_header[0] 
        && header[1] == msg_header[1] 
        && header[2] == msg_header[2] 
        && header[3] == msg_header[3];
}

void loop()
{   
    // Display incomming messages
    //noInterrupts();
    if(last_message.ready)
    {
        bool crc_valid = crc16.checkCRC(reinterpret_cast<uint8_t*>(&last_message.message),
            offsetof(Message, crc16), last_message.message.crc16);

        
        
        char buf[256];
        display.clear();
        if (!crc_valid)
        {
            snprintf(buf, sizeof(buf), "INV. CRC");
        }
        else if(!check_header(last_message.message.header))
        {
            snprintf(buf, sizeof(buf), "INV. HEAD");
        }
        else
        {
            snprintf(buf, sizeof(buf), "PIWO: %i", last_message.message.counter);
            display.drawXbm(128-beer_width, 64-beer_height,
                beer_width, beer_height, inverted_beer_bits);
        }
        display.drawString(0, 0, buf);
        snprintf(buf, sizeof(buf), "RSSI:%i", last_message.RSSI);
        display.drawString(0, 18, buf);
        snprintf(buf, sizeof(buf), "SNR:%.1f", last_message.SNR);
        display.drawString(0, 36, buf);
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
    LoRa.readBytes(static_cast<uint8_t*>(buffer),static_cast<size_t>(sizeof(Message)));

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
