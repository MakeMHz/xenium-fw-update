#pragma once
#include <stdio.h>
#include <stdint.h>

uint32_t CRC_init();
uint32_t CRC_addByte(uint32_t crc, uint8_t b);
uint32_t CRC_add(uint32_t crc, uint8_t const* data, size_t dataLen);
uint32_t CRC_result(uint32_t crc);
