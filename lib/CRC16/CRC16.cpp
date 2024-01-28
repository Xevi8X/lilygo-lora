#include "CRC16.h"

CRC16::CRC16(uint16_t polynomial) {
    initializeTable(polynomial);
}

uint16_t CRC16::calculateCRC(const uint8_t* data, size_t length) {
    uint16_t crc = 0xFFFF; // Initial value

    for (size_t i = 0; i < length; ++i) {
        crc = (crc >> 8) ^ crcTable[(crc ^ data[i]) & 0xFF];
    }

    return crc;
}

bool CRC16::checkCRC(const uint8_t* data, size_t length, uint16_t receivedCRC) {
    uint16_t calculatedCRC = calculateCRC(data, length);
    return calculatedCRC == receivedCRC;
}

void CRC16::initializeTable(uint16_t polynomial) {

    for (uint16_t i = 0; i < 256; ++i) {
        uint16_t crc = i;
        for (int j = 0; j < 8; ++j) {
            crc = (crc & 1) ? (crc >> 1) ^ polynomial : (crc >> 1);
        }
        crcTable[i] = crc;
    }
}