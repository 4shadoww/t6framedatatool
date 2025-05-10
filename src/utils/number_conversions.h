#ifndef NUMBER_CONVERSIONS_H
#define NUMBER_CONVERSIONS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

int32_t big32_to_little(const char * const buf);
int16_t big16_to_little(const char * const buf);
uint64_t ps3_address_to_x64(const uint32_t address);
float big32_to_little_float(const char * const buf);

#ifdef __cplusplus
};
#endif

#endif
