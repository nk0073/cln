/*
 *  Compact Literal Notation
 *  
 *
 *
 *
 */


#ifndef _CLN_IMPORTS
#define _CLN_IMPORTS

#define CLN_SUCCESS 0
#define CLN_OUT_OF_BOUNDS -2

#define CLN_RAW_SEPARATOR ','
#define CLN_INT64_MAX_LENGTH 22
#define CLN_UINT64_MAX_LENGTH 21
#define CLN_FLOAT_MAX_LENGTH 50
#define CLN_DOUBLE_MAX_LENGTH 350

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#ifdef __cplusplus
    #include <cstdint>
    #include <cstddef>
    #include <cstring>
#else
    #include <stdint.h>
    #include <stddef.h>
    #include <string.h>
#endif

#endif

// Important to remember that the allocation is linear
typedef struct {
    char* str; // data in string format
    size_t size; // how many bytes (chars) is used
    size_t capacity; // max size
} cln_buffer;

#ifdef CLN_IMPLEMENTATION // TODO: change to ifndef

// declarations

#else

cln_buffer cln_create_buffer(size_t bytes) {
    if(bytes == 0) bytes = 1;
    char* str = malloc(bytes);
    str[bytes - 1] = '\0';
    return (cln_buffer) {
        .str = str,
        .size = 0,
        .capacity = bytes,
    };
}

void __cln_buffer_increase_capacity(cln_buffer* buffer, const size_t needed_capacity) {
    int8_t increased = 0;
    while(buffer->capacity < needed_capacity) {
        buffer->capacity *= 2;
        increased = 1;
    }

    if(increased) {
        buffer->str = realloc(buffer->str, buffer->capacity + 1);
    }
}

// TODO: dont realloc every time, realloc to twice the size if needed 
int32_t __cln_add_char_ptr(cln_buffer* buffer, const char* str) {
    if(buffer->size > 0) {
        const size_t temp_str_len = strlen(str) + 2; // + 2 because \0 and the separator symbol
        char* temp_str;
        temp_str = malloc(temp_str_len);
        __cln_buffer_increase_capacity(buffer, buffer->size + temp_str_len);
        strcpy(temp_str, (char[]){CLN_RAW_SEPARATOR, '\0'});
        strcat(temp_str, str);
        strcat(buffer->str, temp_str);
        buffer->size += temp_str_len - 1;
        free(temp_str);
    } else {
        __cln_buffer_increase_capacity(buffer, strlen(str) + 1);
        buffer->size = strlen(str);
        strcpy(buffer->str, str);
    }
    
    return CLN_SUCCESS;
}

int32_t cln_add_str(cln_buffer* buffer, const char* str, const size_t bytes) {
    // count the characters that would be formatted as separators
    uint32_t unformatted_sep_count = 0;
    for(const char* c = str; c < str + bytes; c++) {
        if(c[0] == CLN_RAW_SEPARATOR) {
            unformatted_sep_count++;
        }
    }

    int32_t exit_code;
    if(unformatted_sep_count > 0) { // if there are separator characters, replace them
        uint32_t len = bytes + unformatted_sep_count;
        char* new_str = malloc(len + 1);
        uint32_t src_str_index = 0;
        for(uint32_t i = 0; i < len; i++) {
            const char* src_c = str + src_str_index;
            char* c = new_str + i;
            if(*src_c == CLN_RAW_SEPARATOR) {
                *c = '\\';
                *(c + 1) = CLN_RAW_SEPARATOR;
                i++;
            } else {
                *c = *src_c;
            }

            src_str_index++;
        }
        new_str[len] = '\0';
        exit_code = __cln_add_char_ptr(buffer, new_str);
        free(new_str);
    } else {
        exit_code = __cln_add_char_ptr(buffer, str);
    }

    return exit_code;
}

int32_t cln_add_int(cln_buffer* buffer, int64_t value) {
    char str[CLN_INT64_MAX_LENGTH];
    snprintf(str, CLN_INT64_MAX_LENGTH, "%ld", value);

    return __cln_add_char_ptr(buffer, str);
}

int32_t cln_add_uint(cln_buffer* buffer, const uint64_t value) {
    char str[CLN_UINT64_MAX_LENGTH];
    snprintf(str, CLN_UINT64_MAX_LENGTH, "%ld", value);

    return __cln_add_char_ptr(buffer, str);
}

int32_t cln_add_float(cln_buffer* buffer, const float value, const char* format) {
    char str[CLN_FLOAT_MAX_LENGTH];
    snprintf(str, CLN_FLOAT_MAX_LENGTH, format, value);

    return __cln_add_char_ptr(buffer, str);
}

int32_t cln_add_double(cln_buffer* buffer, const double value, const char* format) {
    char str[CLN_DOUBLE_MAX_LENGTH];
    snprintf(str, CLN_DOUBLE_MAX_LENGTH, format, value);

    return __cln_add_char_ptr(buffer, str);
}

void cln_free_buffer(cln_buffer* buffer) {
    free(buffer->str);
}

void cln_trim_capacity(cln_buffer* buffer) {
    if(buffer->capacity == buffer->size + 1) return;
    buffer->capacity = buffer->size;
    buffer->str = realloc(buffer->str, buffer->capacity + 1);
}

#endif
