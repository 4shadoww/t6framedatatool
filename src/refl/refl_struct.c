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

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "logging.h"
#include "number_conversions.h"
#include "refl_struct.h"

static int field_in_range(const struct Field *field, size_t size) {
    if (field->width != 2 && field->width != 4 && field->width != 8) {
        log_error("field %s has width %u", field->name, field->width);
        return 0;
    }
    if ((size_t) field->offset + field->width > size) {
        log_error("field %s extends past the struct", field->name);
        return 0;
    }
    return 1;
}

static int swap_field(unsigned char *bytes, const struct Field *field) {
    char *src = (char *) bytes;

    if (field->width == 2) {
        const int16_t value = big16_to_little(src);
        memcpy(bytes, &value, sizeof(value)); // NOLINT
    } else if (field->width == 4) {
        const int32_t value = big32_to_little(src);
        memcpy(bytes, &value, sizeof(value)); // NOLINT
    } else if (field->width == 8) {
        const int64_t value = big64_to_little(src);
        memcpy(bytes, &value, sizeof(value)); // NOLINT
    } else {
        return REFL_ERROR;
    }
    return REFL_OK;
}

int load_refl_struct(void *out, const void *memory, size_t size, const struct Field *fields, size_t field_count) {
    unsigned char *bytes = NULL;

    if (out == NULL || memory == NULL || size == 0 || (field_count > 0 && fields == NULL)) {
        log_error("load_refl_struct called with no struct");
        return REFL_ERROR;
    }

    memcpy(out, memory, size); // NOLINT
    bytes = out;
    for (size_t i = 0; i < field_count; i++) {
        if (!field_in_range(&fields[i], size)) {
            return REFL_ERROR;
        }
        if (fields[i].swap == 0) {
            continue;
        }
        if (swap_field(bytes + fields[i].offset, &fields[i]) != REFL_OK) {
            return REFL_ERROR;
        }
    }
    return REFL_OK;
}

static int append_text(char *out, size_t out_size, size_t *used, const char *fmt, ...) {
    va_list args;
    char *dest = NULL;
    size_t space = 0;

    if (*used < out_size) {
        dest = out + *used;
        space = out_size - *used;
    }

    va_start(args, fmt);
    const int wrote = vsnprintf(dest, space, fmt, args); // NOLINT
    va_end(args);
    if (wrote < 0) {
        return REFL_ERROR;
    }

    *used += (size_t) wrote;
    return REFL_OK;
}

static int
append_field(char *out, size_t out_size, size_t *used, const struct Field *field, const unsigned char *bytes) {
    if (field->swap == 0) {
        if (field->width == 2) {
            return append_text(out, out_size, used, "    %s = %02x %02x\n", field->name, bytes[0], bytes[1]);
        }
        if (field->width == 4) {
            return append_text(out,
                               out_size,
                               used,
                               "    %s = %02x %02x %02x %02x\n",
                               field->name,
                               bytes[0],
                               bytes[1],
                               bytes[2],
                               bytes[3]);
        }
        return append_text(out,
                           out_size,
                           used,
                           "    %s = %02x %02x %02x %02x %02x %02x %02x %02x\n",
                           field->name,
                           bytes[0],
                           bytes[1],
                           bytes[2],
                           bytes[3],
                           bytes[4],
                           bytes[5],
                           bytes[6],
                           bytes[7]);
    }

    if (field->width == 2) {
        uint16_t value = 0;
        memcpy(&value, bytes, sizeof(value)); // NOLINT
        return append_text(out, out_size, used, "    %s = %" PRIu16 " (0x%04" PRIx16 ")\n", field->name, value, value);
    }
    if (field->width == 4) {
        uint32_t value = 0;
        memcpy(&value, bytes, sizeof(value)); // NOLINT
        return append_text(out, out_size, used, "    %s = %" PRIu32 " (0x%08" PRIx32 ")\n", field->name, value, value);
    }

    uint64_t value = 0;
    memcpy(&value, bytes, sizeof(value)); // NOLINT
    return append_text(out, out_size, used, "    %s = %" PRIu64 " (0x%" PRIx64 ")\n", field->name, value, value);
}

int format_refl_struct(char *out,
                       size_t out_size,
                       const char *name,
                       const void *data,
                       const struct Field *fields,
                       size_t field_count) {
    const unsigned char *bytes = NULL;
    size_t used = 0;

    if (out == NULL || out_size == 0 || data == NULL || (field_count > 0 && fields == NULL)) {
        return REFL_ERROR;
    }

    bytes = data;
    if (append_text(out, out_size, &used, "%s\n", name != NULL ? name : "") != REFL_OK) {
        return REFL_ERROR;
    }

    for (size_t i = 0; i < field_count; i++) {
        if (fields[i].width != 2 && fields[i].width != 4 && fields[i].width != 8) {
            return REFL_ERROR;
        }
        if (append_field(out, out_size, &used, &fields[i], bytes + fields[i].offset) != REFL_OK) {
            return REFL_ERROR;
        }
    }

    return (int) used;
}
