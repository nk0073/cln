/*
 *  Compact Literal Notation
 *  
 *  Right now it's only possible to encode the data without layout
 *
 *  todos (in order)
 *  TODO: add functionality to layout such as reading data from the buffer
 *  TODO: implement decoding
 *  TODO: add tests
 *  TODO: make a proper error system
 *  TODO: implement arrays
 *  TODO: implement objets
 */

#ifndef _CLN_IMPORTS
#define _CLN_IMPORTS

#define CLN_SUCCESS 0
#define CLN_ERROR_OUT_OF_BOUNDS -1
#define CLN_ERROR_INVALID_POINTER -2
#define CLN_ERROR_EMPTY_LAOUT -3

#define CLN_RAW_SEPARATOR ','
#define CLN_INT64_MAX_LENGTH 22
#define CLN_UINT64_MAX_LENGTH 21
#define CLN_FLOAT_MAX_LENGTH 50
#define CLN_DOUBLE_MAX_LENGTH 350

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>   

#ifdef __cplusplus
    #include <cstdint>
    #include <cstddef>
    #include <cstring>
    #include <cstdarg>
#else
    #include <stdint.h>
    #include <stddef.h>
    #include <string.h>
    #include <stdarg.h>
#endif

#endif

#define CLN_ERROR_SIZE 1024
char cln_last_error[CLN_ERROR_SIZE];

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
    char* str; // data in string format
    size_t size; // how many bytes (chars) is used
    size_t capacity; // max size
    
    // can be 0 until needed
    cln_layout* layout;
    size_t layout_size;
    int32_t err;
} cln_buffer;

#ifdef CLN_IMPLEMENTATION // TODO: change to ifndef when done

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
        .err = 0,
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
    
    return CLN_SUCCESS;
}

int32_t cln_add_str(cln_buffer* buffer, const char* str, const size_t bytes) {
    // count the characters that would be formatted as separators
    uint32_t unformatted_sep_count = 0;
    for(const char* c = str + 1; c < str + bytes; ++c) {
        if(c[0] != '\\' && c[1] == CLN_RAW_SEPARATOR) {
            unformatted_sep_count++;
        }
    }

    int32_t exit_code;
    if(unformatted_sep_count > 0) { // if there are separator characters, replace them
        uint32_t len = bytes + unformatted_sep_count;
        char* new_str = malloc(len + 1);
        *new_str = *str;
        uint32_t src_str_index = 0;
        for(uint32_t i = 1; i < len; ++i) {
            const char* src_c = str + src_str_index;
            char* c = new_str + i;
            if(*src_c == CLN_RAW_SEPARATOR && *(src_c - 1) != '\\') {
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

void __cln_report_snprintf_err(void) {
    switch (errno) {
        case EOVERFLOW:
            snprintf(cln_last_error, CLN_ERROR_SIZE, "snprintf error: Output was truncated and could not fit in the buffer.\n");
            break;
        case EILSEQ:
            snprintf(cln_last_error, CLN_ERROR_SIZE, "snprintf error: Invalid multibyte sequence encountered.\n");
            break;
        case ENOMEM:
            snprintf(cln_last_error, CLN_ERROR_SIZE, "snprintf error: Not enough memory.\n");
            break;
        default:
            snprintf(cln_last_error, CLN_ERROR_SIZE, "snprintf unknown error: %s\n", strerror(errno));
    }
}

int32_t cln_add_int(cln_buffer* buffer, int64_t value) {
    char str[CLN_INT64_MAX_LENGTH];
    if(snprintf(str, CLN_INT64_MAX_LENGTH, "%ld", value) < 0) {
        __cln_report_snprintf_err();
    }

    return __cln_add_char_ptr(buffer, str);
}

int32_t cln_add_uint(cln_buffer* buffer, const uint64_t value) {
    char str[CLN_UINT64_MAX_LENGTH];
    if(snprintf(str, CLN_UINT64_MAX_LENGTH, "%ld", value) < 0) {
        __cln_report_snprintf_err();
    }

    return __cln_add_char_ptr(buffer, str);
}

// pattern the same that you would use in printf(pattern, value)
int32_t cln_add_float(cln_buffer* buffer, const float value, const char* format) {
    char str[CLN_FLOAT_MAX_LENGTH];
    if(snprintf(str, CLN_FLOAT_MAX_LENGTH, format, value) < 0) {
        __cln_report_snprintf_err();
    }
    return __cln_add_char_ptr(buffer, str);
}

// pattern the same that you would use in printf(pattern, value)
int32_t cln_add_double(cln_buffer* buffer, const double value, const char* format) {
    char str[CLN_DOUBLE_MAX_LENGTH];
    if(snprintf(str, CLN_DOUBLE_MAX_LENGTH, format, value) < 0) {
        __cln_report_snprintf_err();
    }

    return __cln_add_char_ptr(buffer, str);
}

cln_buffer cln_read_buffer(const char* cln_source) {
    if(cln_source == NULL) {
        snprintf(cln_last_error, CLN_ERROR_SIZE, "Invalid pointer provided to cln_read_buffer\n");
        return (cln_buffer) {
            .err = CLN_ERROR_INVALID_POINTER,
        };
    }

    size_t len = strlen(cln_source);
    cln_buffer buffer = cln_create_buffer(len + 1);
    buffer.size = len;

    return buffer;
}

int32_t cln_set_layout(cln_buffer* buffer, const cln_layout* layout, const size_t layout_size) {
    if(layout == NULL) {
        snprintf(cln_last_error, CLN_ERROR_SIZE, "Invalid layout pointer provided to cln_set_layout\n");
        return CLN_ERROR_INVALID_POINTER;
    } if(layout_size == 0) {
        snprintf(cln_last_error, CLN_ERROR_SIZE, "layout_size of size 0 was provided to cln_set_layout\n");
        return CLN_ERROR_EMPTY_LAOUT;
    }
    if(buffer->layout != NULL) {
        free(buffer->layout);
    }
    
    buffer->layout = malloc(sizeof(cln_layout) * layout_size);
    memcpy(buffer->layout, layout, layout_size * sizeof(cln_layout));
    buffer->layout_size = layout_size; 

    return CLN_SUCCESS;
}

void cln_free_buffer(cln_buffer* buffer) {
    free(buffer->str);
    free(buffer->layout);
}

void cln_trim_capacity(cln_buffer* buffer) {
    if(buffer->capacity == buffer->size + 1) return;
    buffer->capacity = buffer->size;
    buffer->str = realloc(buffer->str, buffer->capacity + 1);
}

#endif
