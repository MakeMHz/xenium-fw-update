#pragma once
#include <stdint.h>

typedef struct Version {
    char *name;
    uint32_t crc32_xos;
    uint32_t crc32_bootloader;
} Version_t;

extern const Version XeniumVersions[5];
