/*
  Copyright (C) 2026 Noa-Emil Nissinen

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.    If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef REFL_STRUCT_H
#define REFL_STRUCT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#define REFL_OK (0)
#define REFL_ERROR (-1)

/* One member of a reflected struct.
 * width is 2, 4, or 8. swap 0 leaves those guest bytes in place. */
struct Field {
    const char *name;
    uint16_t offset;
    uint8_t width;
    uint8_t swap;
};

#define FIELD_ENTRY(type, member, field_width, field_swap) \
    {#member, (uint16_t) offsetof(type, member), (uint8_t) (field_width), (uint8_t) (field_swap)}

#define F16(type, member, field_swap) FIELD_ENTRY(type, member, 2, field_swap)
#define F32(type, member, field_swap) FIELD_ENTRY(type, member, 4, field_swap)
#define F64(type, member, field_swap) FIELD_ENTRY(type, member, 8, field_swap)

/* Copy guest bytes from memory into out, then byteswap fields marked for swap. */
int load_refl_struct(void *out, const void *memory, size_t size, const struct Field *fields, size_t field_count);

/* Write the struct into out. Returns the character count excluding the terminating NUL.
 * A return value >= out_size means the text was cut to fit. REFL_ERROR means the arguments are invalid. */
int format_refl_struct(char *out,
                       size_t out_size,
                       const char *name,
                       const void *data,
                       const struct Field *fields,
                       size_t field_count);

#ifdef __cplusplus
}
#endif

#endif
