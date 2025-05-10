#include <stdint.h>

#include "number_conversions.h"

int32_t big32_to_little(const char * const buf) {
    union {
        char bytes[4];
        int32_t value;
    } u;

    u.bytes[0] = buf[3];
    u.bytes[1] = buf[2];
    u.bytes[2] = buf[1];
    u.bytes[3] = buf[0];

    return u.value;
}

int16_t big16_to_little(const char * const buf) {
    union {
        char bytes[2];
        int16_t value;
    } u;

    u.bytes[0] = buf[1];
    u.bytes[1] = buf[0];

    return u.value;
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
