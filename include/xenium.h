/*
 * Xenium-Tools - https://raw.githubusercontent.com/Ryzee119/Xenium-Tools/master/xenium/xenium.h
 *
 * Copyright (c) 2021 Ryzee119
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#ifdef NXDK
#define LPC_MEMORY_BASE 0xFF000000u
#endif

#define XENIUM_REGISTER_BANKING 0x00EF
#define XENIUM_REGISTER_LED 0x00EE

#define XENIUM_LED_OFF    0
#define XENIUM_LED_RED    1
#define XENIUM_LED_GREEN  2
#define XENIUM_LED_AMBER  3
#define XENIUM_LED_BLUE   4
#define XENIUM_LED_PURPLE 5
#define XENIUM_LED_TEAL   6
#define XENIUM_LED_WHITE  7

#define XENIUM_MANUF_ID  0x01
#define XENIUM_DEVICE_ID 0xC4
#define XENIUM_FLASH_NUM_SECTORS 35
#define XENIUM_FLASH_SECTOR_SIZE (0x10000)
#define XENIUM_FLASH_SIZE (1024 * 1024 * 2)

#define XENIUM_BANK_TSOP        0
#define XENIUM_BANK_BOOTLOADER  1
#define XENIUM_BANK_XENIUMOS    2
#define XENIUM_BANK_1           3
#define XENIUM_BANK_2           4
#define XENIUM_BANK_3           5
#define XENIUM_BANK_4           6
#define XENIUM_BANK_1_512       7
#define XENIUM_BANK_2_512       8
#define XENIUM_BANK_1_1024      9
#define XENIUM_BANK_RECOVERY    10

#define XENIUM_BANK_BOOTLOADER_SIZE  256 * 1024
#define XENIUM_BANK_XENIUMOS_SIZE    512 * 1024

#define XENIUM_BANK_BOOTLOADER_OFFSET  0x180000
#define XENIUM_BANK_XENIUMOS_OFFSET    0x100000

void xenium_set_bank(uint8_t bank);
void xenium_set_led(uint8_t led);
uint8_t xenium_get_bank(void);
uint8_t xenium_is_detected(void);
uint8_t xenium_flash_busy(void);

void xenium_start_sector_erase(uint32_t sector_address);
void xenium_start_flash_program_byte(uint32_t address, uint8_t data);
void xenium_flash_read_stream(uint32_t address, uint8_t *data, uint32_t len);
uint32_t xenium_sector_to_address(uint8_t sector);
uint32_t xenium_sector_to_size(uint8_t sector);
uint32_t xenium_sector_to_bank(uint8_t sector);
void xenium_flash_reset(void);
void xenium_set_working_buffer(uint8_t *buffer);
