/*
 *  Compact Literal Notation
 *
 *  Right now it's only possible to encode the data without layout
 *
 *  TODO: fix __cln_alloc_get_by_index empty string issues
 *  TODO: make production ready
 *  TODO: add documentation
 *  TODO: perhaps add more error codes somewhere
 */

#ifndef _CLN_IMPORTS
#define _CLN_IMPORTS

#define CLN_SUCCESS 0
#define CLN_EMPTY_STRING 1
// to get more info about errors, call cln_get_last_err()
#define CLN_ERROR_OUT_OF_BOUNDS -1
#define CLN_ERROR_INVALID_POINTER -2
#define CLN_ERROR_EMPTY_LAYOUT -3
#define CLN_ERROR_EMPTY_STRING -4
#define CLN_ERROR_SNPRINTF -5

#define CLN_RAW_SEPARATOR ','
#define CLN_INT64_MAX_LENGTH 22
#define CLN_UINT64_MAX_LENGTH 21
#define CLN_FLOAT_MAX_LENGTH 50
#define CLN_DOUBLE_MAX_LENGTH 350

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstring>
#else
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#endif

#endif

#define CLN_ERROR_SIZE 1024
static char cln_last_error[CLN_ERROR_SIZE];
char* cln_get_last_err(void) { return cln_last_error; }
#define __CLN_ERROR(x) snprintf(cln_last_error, CLN_ERROR_SIZE, x)
#define __CLN_ERROR_ARGS(x, ...) snprintf(cln_last_error, CLN_ERROR_SIZE, x, __VA_ARGS__)

typedef enum {
    CLN_STRING,

    CLN_INT8,
    CLN_INT16,
    CLN_INT32,
    CLN_INT64,

    CLN_UINT8,
    CLN_UINT16,
    CLN_UINT32,
    CLN_UINT64,

    CLN_FLOAT,
    CLN_DOUBLE
} cln_layout;

// Important to remember that the allocation is linear
typedef struct {
    // non nullable
    char* str;       // data in string format
    size_t size;     // how many bytes (chars) is used
    size_t capacity; // max size EXCLUDING \0
    size_t count;    // the amount of items added

    // nullable
    cln_layout* layout;
    size_t layout_len;
    int32_t err;

    void** allocated_vars; // all allocated variables by cln_retrieve items
    size_t allocated_len;
} cln_buffer;

#ifdef CLN_IMPLEMENTATION // TODO: change to ifndef when done

// declarations

#else

cln_buffer cln_create_buffer(size_t bytes) {
    if(bytes == 0)
        bytes = 1;
    char* str = malloc(bytes);
    str[bytes - 1] = '\0';
    return (cln_buffer){
        .str = str,
        .size = 0,
        .capacity = bytes - 1,
        .count = 0,

        .layout = NULL,
        .layout_len = 0,
        .err = 0,

        .allocated_vars = NULL,
        .allocated_len = 0,
    };
}

void __cln_buffer_increase_capacity(cln_buffer* buffer, const size_t needed_capacity) {
    int8_t increased = 0;
    if(buffer->capacity == 0 && needed_capacity > 0) {
        buffer->capacity++;
    }

    while(buffer->capacity < needed_capacity) {
        buffer->capacity *= 2;
        increased = 1;
    }

    if(increased) {
        buffer->str = realloc(buffer->str, buffer->capacity + 1);
    }
}

int32_t __cln_add_char_ptr(cln_buffer* buffer, const char* str) {
    if(buffer->size > 0) {
        // + 2 because \0 and the separator symbol
        const size_t temp_str_len = strlen(str) + 2;
        char* temp_str;
        temp_str = malloc(temp_str_len);
        __cln_buffer_increase_capacity(buffer, buffer->size + temp_str_len - 1);
        strcpy(temp_str, (char[]){CLN_RAW_SEPARATOR, '\0'});
        strcat(temp_str, str);
        strcat(buffer->str, temp_str);
        buffer->size += temp_str_len - 1;
        free(temp_str);
    } else {
        __cln_buffer_increase_capacity(buffer, strlen(str));
        buffer->size = strlen(str);
        strcpy(buffer->str, str);
    }

    buffer->count++;
    return CLN_SUCCESS;
}

int32_t cln_add_str(cln_buffer* buffer, const char* str, const size_t bytes) {
    if(bytes == 0) {
        __CLN_ERROR("Empty string passed to cln_add_str");
        return __cln_add_char_ptr(buffer, "");
    }

    // count the characters that would be read as separators
    uint32_t unformatted_sep_count = 0;
    if(str[0] == CLN_RAW_SEPARATOR) {
        unformatted_sep_count++;
    }

    for(const char* c = str; c < str + bytes; ++c) {
        if(c[0] != '\\' && c[1] == CLN_RAW_SEPARATOR) {
            unformatted_sep_count++;
        }
    }

    int32_t exit_code;
    // if there are separator characters, replace them
    if(unformatted_sep_count > 0) {
        uint32_t len = bytes + unformatted_sep_count;
        char* new_str = malloc(len + 1);
        size_t i = 0;
        size_t j = 0;
        for(; j < len; ++i, ++j) {
            const char* og = str + i;
            char* n = new_str + j;
            if(og[0] == CLN_RAW_SEPARATOR && ((i > 0 && og[-1] != '\\') || i == 0)) {
                n[0] = '\\';
                n[1] = CLN_RAW_SEPARATOR;
                j++;
            } else {
                n[0] = og[0];
            }
        }

        new_str[len] = '\0';
        exit_code = __cln_add_char_ptr(buffer, new_str);
        free(new_str);
    } else {
        exit_code = __cln_add_char_ptr(buffer, str);
    }

    return exit_code;
}

int32_t __cln_report_snprintf_err(void) {
    switch(errno) {
    case EOVERFLOW:
        __CLN_ERROR("snprintf error: Output was truncated and could not fit in "
                    "the buffer.\n");
        break;
    case EILSEQ:
        __CLN_ERROR("snprintf error: Invalid multibyte sequence encountered.\n");
        break;
    case ENOMEM:
        __CLN_ERROR("snprintf error: Not enough memory.\n");
        break;
    default:
        __CLN_ERROR_ARGS("snprintf unknown error: %s\n", strerror(errno));
    }

    return CLN_ERROR_SNPRINTF;
}

int32_t cln_add_int(cln_buffer* buffer, int64_t value) {
    char str[CLN_INT64_MAX_LENGTH];
    if(snprintf(str, CLN_INT64_MAX_LENGTH, "%" SCNd64, value) < 0) {
        return __cln_report_snprintf_err();
    }

    return __cln_add_char_ptr(buffer, str);
}

int32_t cln_add_uint(cln_buffer* buffer, const uint64_t value) {
    char str[CLN_UINT64_MAX_LENGTH];
    if(snprintf(str, CLN_UINT64_MAX_LENGTH, "%" SCNu64, value) < 0) {
        return __cln_report_snprintf_err();
    }

    return __cln_add_char_ptr(buffer, str);
}

// pattern is the same that you would use in printf(pattern, value)
int32_t cln_add_float(cln_buffer* buffer, const float value, const char* format) {
    char str[CLN_FLOAT_MAX_LENGTH];
    if(snprintf(str, CLN_FLOAT_MAX_LENGTH, format, value) < 0) {
        return __cln_report_snprintf_err();
    }
    return __cln_add_char_ptr(buffer, str);
}

// pattern the same that you would use in printf(pattern, value)
int32_t cln_add_double(cln_buffer* buffer, const double value, const char* format) {
    char str[CLN_DOUBLE_MAX_LENGTH];
    if(snprintf(str, CLN_DOUBLE_MAX_LENGTH, format, value) < 0) {
        return __cln_report_snprintf_err();
    }

    return __cln_add_char_ptr(buffer, str);
}

cln_buffer cln_read_buffer(const char* cln_source) {
    if(cln_source == NULL) {
        __CLN_ERROR("Invalid pointer provided to cln_read_buffer\n");
        return (cln_buffer){
            .err = CLN_ERROR_INVALID_POINTER,
        };
    }

    size_t len = strlen(cln_source);
    cln_buffer buffer = cln_create_buffer(len + 1);
    buffer.size = len;
    strcpy(buffer.str, cln_source);

    return buffer;
}

int32_t cln_set_layout(cln_buffer* buffer, const cln_layout* layout, const size_t layout_len) {
    if(layout == NULL) {
        __CLN_ERROR("Invalid layout pointer provided to cln_set_layout\n");
        return CLN_ERROR_INVALID_POINTER;
    }
    if(layout_len == 0) {
        __CLN_ERROR("layout_size of size 0 was provided to cln_set_layout\n");
        return CLN_ERROR_EMPTY_LAYOUT;
    }

    const size_t layout_bytes = sizeof(cln_layout) * layout_len;
    const size_t allocator_bytes = sizeof(void*) * layout_len;
    if(buffer->layout == NULL) {
        buffer->layout = malloc(layout_bytes);
    } else {
        buffer->layout = realloc(buffer->layout, layout_bytes);
    }
    memcpy(buffer->layout, layout, layout_bytes);

    if(buffer->allocated_vars == NULL) {
        buffer->allocated_vars = malloc(allocator_bytes);
    } else {
        buffer->allocated_vars = realloc(buffer->allocated_vars, allocator_bytes);
    }

    buffer->layout_len = layout_len;
    buffer->allocated_len = layout_len;

    return CLN_SUCCESS;
}

int32_t __cln_alloc_get_by_index(cln_buffer* buffer, const size_t index, void** out) {
    size_t current_index = 0;
    uint32_t start = 0;
    uint32_t len = 0;
    for(size_t i = 0; i < buffer->size; ++i, ++len) {
        const char* c = buffer->str + i;
        if(c[0] == CLN_RAW_SEPARATOR && ((i > 0 && c[-1] != '\\') || i == 0)) {
            if(current_index == index) {
                break;
            }

            len = 0;
            start = i + 1;
            current_index++;
        }
    }

    if(len == 0 || (buffer->str + start)[0] == CLN_RAW_SEPARATOR) {
        *out = malloc(1);
        strcpy(*out, "");
        return CLN_EMPTY_STRING;
    }

    char* str = malloc(len + 1);
    memcpy(str, buffer->str + start, len);
    str[len] = '\0';
    switch(buffer->layout[index]) {
    case CLN_STRING:
        *out = str;
        break;
    case CLN_INT8: {
        *out = malloc(sizeof(int8_t));
        sscanf(str, "%" SCNd8, (int8_t*)*out);
        break;
    }
    case CLN_INT16: {
        *out = malloc(sizeof(int16_t));
        sscanf(str, "%" SCNd16, (int16_t*)*out);
        break;
    }
    case CLN_INT32: {
        *out = malloc(sizeof(int32_t));
        sscanf(str, "%" SCNd32, (int32_t*)*out);

        break;
    }
    case CLN_INT64: {
        *out = malloc(sizeof(int64_t));
        sscanf(str, "%" SCNd64, (int64_t*)*out);
        break;
    }
    case CLN_UINT8: {
        *out = malloc(sizeof(uint8_t));
        sscanf(str, "%" SCNu8, (uint8_t*)*out);
        break;
    }
    case CLN_UINT16: {
        *out = malloc(sizeof(uint16_t));
        sscanf(str, "%" SCNu16, (uint16_t*)*out);
        break;
    }
    case CLN_UINT32: {
        *out = malloc(sizeof(uint32_t));
        sscanf(str, "%" SCNu32, (uint32_t*)*out);
        break;
    }
    case CLN_UINT64: {
        *out = malloc(sizeof(uint64_t));
        sscanf(str, "%" SCNu64, (uint64_t*)*out);
        break;
    }
    case CLN_FLOAT: {
        *out = malloc(sizeof(float));
        sscanf(str, "%f", (float*)*out);
        break;
    }
    case CLN_DOUBLE: {
        *out = malloc(sizeof(double));
        sscanf(str, "%lf", (double*)*out);
        break;
    }
    }

    if(buffer->layout[index] != CLN_STRING)
        free(str);

    return CLN_SUCCESS;
}

// all va_args should be void** or similar
int32_t cln_retrieve_items(cln_buffer* buffer, ...) {
    va_list args;
    va_start(args, buffer);
    int32_t last_result = CLN_SUCCESS;
    for(size_t x = 0; x < buffer->layout_len; x++) {
        void** arg = va_arg(args, void**);
        if(arg == NULL) {
            buffer->allocated_vars[x] = NULL;
            continue;
        }

        last_result = __cln_alloc_get_by_index(buffer, x, arg);
        buffer->allocated_vars[x] = *arg;
        if(last_result < 0) {
            return last_result;
        }
    }
    return last_result;
}

// the caller needs to free z themselves
#define cln_alloc_get_by_index(x, y, z) __cln_alloc_get_by_index(x, y, &z)

void cln_free_buffer(cln_buffer* buffer) {
    free(buffer->str);
    free(buffer->layout);
    for(size_t x = 0; x < buffer->allocated_len; ++x) {
        void* __tmp = buffer->allocated_vars[x];
        free(__tmp);
        // free(buffer->allocated_vars[x]);
    }

    free(buffer->allocated_vars);
}

void cln_trim_capacity(cln_buffer* buffer) {
    if(buffer->capacity == buffer->size)
        return;

    buffer->capacity = buffer->size;
    buffer->str = realloc(buffer->str, buffer->capacity + 1);
}

#endif
