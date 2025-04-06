#include "number_conversions.h"

uint32_t big32_to_little(const char * const buf) {
    uint32_t value = buf[3] & 0xff;
    value |= (buf[2] << 8) & 0xff00;
    value |= (buf[1] << 16) & 0xff00;
    value |= (buf[0] << 24) & 0xff0000;

    return value;
}
