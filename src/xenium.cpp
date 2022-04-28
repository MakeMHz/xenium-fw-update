/*
 * Xenium-Tools - https://raw.githubusercontent.com/Ryzee119/Xenium-Tools/master/xenium/xenium.c
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
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "xenium.h"

#ifdef NXDK
static unsigned char IoInputByte(unsigned short address)
{
	unsigned char v;
	__asm__ __volatile__ ("inb %w1,%0":"=a" (v):"Nd" (address));
	return v;
}

static void IoOutputByte(unsigned short address, unsigned char value)
{
	__asm__ __volatile__ ("outb %b0,%w1": :"a" (value), "Nd" (address));
}
#else
static uint8_t lpc_mem[1024 * 1024 * 16];
static uintptr_t LPC_MEMORY_BASE;
#endif

//See https://github.com/Ryzee119/OpenXenium/blob/master/Firmware/openxenium.vhd
//for Xenium CPLD details

static void _io_output(uint16_t address, uint8_t value)
{
#ifdef NXDK
    IoOutputByte(address, value);
#else
//printf("_io_output: 0x%04x, 0x%02x\n", address, value);
#endif
}

static uint8_t _io_input(uint16_t address)
{
#ifdef NXDK
    return IoInputByte(address);
#else
//printf("_io_input: 0x%04x\n", address);
#endif
}

//IO CONTROL INTERFACE
void xenium_set_bank(uint8_t bank)
{
    if (bank <= 10)
        _io_output(XENIUM_REGISTER_BANKING, bank);
}

uint8_t xenium_get_bank()
{
    return _io_input(XENIUM_REGISTER_BANKING) & 0x0F;
}

void xenium_set_led(uint8_t led)
{
    _io_output(XENIUM_REGISTER_LED, led);
}

//FLASH MEMORY INTERFACE
static void lpc_send_byte(uint32_t address, uint8_t data)
{
#ifndef NXDK
    LPC_MEMORY_BASE = (uintptr_t)lpc_mem;
#endif
    volatile uint8_t *volatile lpc_mem_map = (uint8_t *)LPC_MEMORY_BASE;
    lpc_mem_map[address] = data;
}

static uint8_t xenium_flash_read_byte(uint32_t address)
{
#ifndef NXDK
    LPC_MEMORY_BASE = (uintptr_t)lpc_mem;
#endif
    volatile uint8_t *volatile lpc_mem_map = (uint8_t *)LPC_MEMORY_BASE;
    return lpc_mem_map[address];
}

void xenium_flash_reset(void)
{
    lpc_send_byte(0x00000000, 0xF0);
}

void xenium_flash_read_stream(uint32_t address, uint8_t *data, uint32_t len)
{
#ifndef NXDK
    LPC_MEMORY_BASE = (uintptr_t)lpc_mem;
#endif
    volatile uint8_t *volatile lpc_mem_map = (uint8_t *)LPC_MEMORY_BASE;
    memcpy(data, (void *)&lpc_mem_map[address], len);
}

uint8_t xenium_flash_busy(void)
{
    return xenium_flash_read_byte(0) != xenium_flash_read_byte(0);
}

void xenium_start_sector_erase(uint32_t sector_address)
{
    lpc_send_byte(0xAAAA, 0xAA);
    lpc_send_byte(0x5555, 0x55);
    lpc_send_byte(0xAAAA, 0x80);
    lpc_send_byte(0xAAAA, 0xAA);
    lpc_send_byte(0x5555, 0x55);
    lpc_send_byte(sector_address, 0x30);
}

void xenium_start_flash_program_byte(uint32_t address, uint8_t data)
{
    lpc_send_byte(0xAAAA, 0xAA);
    lpc_send_byte(0x5555, 0x55);
    lpc_send_byte(0xAAAA, 0xA0);
    lpc_send_byte(address, data);
}

uint8_t xenium_is_detected()
{
    xenium_flash_reset();
    lpc_send_byte(0xAAAA, 0xAA);
    lpc_send_byte(0x5555, 0x55);
    lpc_send_byte(0xAAAA, 0x90);
    uint8_t manuf = xenium_flash_read_byte(0x00);
    xenium_flash_reset();

    lpc_send_byte(0xAAAA, 0xAA);
    lpc_send_byte(0x5555, 0x55);
    lpc_send_byte(0xAAAA, 0x90);
    uint8_t devid = xenium_flash_read_byte(0x02);
    xenium_flash_reset();
    if (manuf == XENIUM_MANUF_ID &&
        devid == XENIUM_DEVICE_ID)
    {
        return 1;
    }
    return 0;
}

uint32_t xenium_sector_to_address(uint8_t sector)
{
    if (sector < 31)  return sector * XENIUM_FLASH_SECTOR_SIZE;
    if (sector == 31) return 0x1F0000;
    if (sector == 32) return 0x1F8000;
    if (sector == 33) return 0x1FA000;
    if (sector == 34) return 0x1FC000;

    return 0;
}

uint32_t xenium_sector_to_size(uint8_t sector)
{
    if (sector < 31)  return 64 * 1024;
    if (sector == 31) return 32 * 1024;
    if (sector == 32) return 8 * 1024;
    if (sector == 33) return 8 * 1024;
    if (sector == 34) return 16 * 1024;

    return 0;
}

uint32_t xenium_sector_to_bank(uint8_t sector)
{
    uint32_t add = xenium_sector_to_address(sector);
    if (add < 0x100000)  return 9;
    if (add < 0x180000)  return 2;
    if (add < 0x1C0000)  return 1;
    return 10;
}
