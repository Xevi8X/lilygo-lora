#pragma once

#include <cstdint>
#include <cstddef>

class CRC16 {
public:
    CRC16(uint16_t polynomial = 0xA001);
    uint16_t calculateCRC(const uint8_t* data, size_t length);
    bool checkCRC(const uint8_t* data, size_t length, uint16_t receivedCRC);

private:
    uint16_t crcTable[256];
    void initializeTable(uint16_t polynomial);
};