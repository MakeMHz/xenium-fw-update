#include <stddef.h>
#include <stdint.h>

static uint32_t revU32(uint32_t d) {
  uint32_t r = 0;
  for (size_t i = 0; i < 32; ++i) {
    r <<= 1;
    r |= (d & 0x01);
    d >>= 1;
  }
  return r;
}

uint32_t CRC_init() { return 0xFFFFFFFF; }

uint32_t CRC_addByte(uint32_t crc, uint8_t b) {
  for (uint8_t pos = 0x01; pos; pos <<= 1) {
    uint32_t msb = crc & 0x80000000;
    if (b & pos) msb ^= 0x80000000;

    if (msb)
      crc = (crc << 1) ^ 0x4C11DB7;
    else
      crc = (crc << 1);
  }
  return crc;
}

uint32_t CRC_add(uint32_t crc, uint8_t const* data, size_t dataLen) {
  for (; dataLen > 0; ++data, --dataLen) {
    crc = CRC_addByte(crc, *data);
  }
  return crc;
}

uint32_t CRC_result(uint32_t crc) { return (revU32(crc) ^ 0xFFFFFFFF); }
