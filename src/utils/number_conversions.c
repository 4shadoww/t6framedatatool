#include <stdint.h>

#include "number_conversions.h"

uint32_t big32_to_little(const char * const buf) {
    uint32_t value = buf[3] & 0xff;
    value |= (buf[2] << 8) & 0xff00;
    value |= (buf[1] << 16) & 0xff00;
    value |= (buf[0] << 24) & 0xff0000;

    return value;
}

uint32_t big16_to_little(const char * const buf) {
    uint32_t value = buf[1] & 0xff;
    value |= (buf[0] << 8) & 0xff00;
    return value;
}

uint64_t ps3_address_to_x64(const uint32_t address) {
    static const uint64_t base_address = 0x300000000;
    static const uint8_t offset = 104;

    return address + base_address + offset;
}

float big32_to_little_float(const char * const buf) {
    union {
        char bytes[4];
        float value;
    } u;

    u.bytes[0] = buf[3];
    u.bytes[1] = buf[2];
    u.bytes[2] = buf[1];
    u.bytes[3] = buf[0];

    return u.value;
}
