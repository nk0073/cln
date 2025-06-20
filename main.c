#define CLN_IMPLEMENTATION
#include "cln.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define SUCCESS_RETURN(x)                                                                                                             \
    cln_free_buffer(&x);                                                                                                              \
    return 0

#define ERROR_RETURN(x)                                                                                                               \
    cln_free_buffer(&x);                                                                                                              \
    return 1

// tests cln_add_str, cln_add_int, cln_trim_capacity
int32_t test_add_buffer(void) {
    cln_buffer buffer = cln_create_buffer(0);
    const char* str = ",,wow,,";
    const int32_t _int = 737373;
    const int32_t str_r = cln_add_str(&buffer, str, strlen(str));
    const int32_t int_r = cln_add_int(&buffer, _int);
    if(str_r < CLN_SUCCESS) {
        const char* err = cln_get_last_err();
        printf("test_add_buffer failed\n  str_r error code: %d\n  error message: %s", str_r, err);
        ERROR_RETURN(buffer);
    }
    if(int_r < CLN_SUCCESS) {
        const char* err = cln_get_last_err();
        printf("test_add_buffer failed\n  int_r error code: %d\n  error message: %s", int_r, err);
        ERROR_RETURN(buffer);
    }

    const char* expected_str = "\\,\\,wow\\,\\,,737373";
    if(strcmp(buffer.str, expected_str)) {
        printf("test_add_buffer failed\n  expected string: %s\n  returned stirng: %s\n", expected_str, buffer.str);
        ERROR_RETURN(buffer);
    }

    cln_trim_capacity(&buffer);
    if(strcmp(buffer.str, expected_str)) {
        printf("test_add_buffer failed\n  before trimming: %s\n  after trimming: %s\n strings do not match", buffer.str, expected_str);
        ERROR_RETURN(buffer);
    }

    SUCCESS_RETURN(buffer);
}

// test cln_add_uint, cln_add_double, cln_add_str for an empty string, cln_set_layout, cln_retrieve_items
int32_t test_layout(void) {
#define __TEST_D_FORMAT "%.17f"
    cln_buffer buffer = cln_create_buffer(12);

    const uint8_t _uint = 66;
    const char* empty_str = "";
    const double _double = 412897124984.89247981249;

    const int32_t uint_r = cln_add_uint(&buffer, _uint);
    const int32_t empty_str_r = cln_add_str(&buffer, empty_str, strlen(empty_str));
    const int32_t double_r = cln_add_double(&buffer, _double, __TEST_D_FORMAT);

    if(uint_r < CLN_SUCCESS) {
        const char* err = cln_get_last_err();
        printf("test_layout failed\n  uint_r error code: %d\n  error message: %s\n", uint_r, err);
        ERROR_RETURN(buffer);
    }
    if(empty_str_r < CLN_SUCCESS && empty_str_r != CLN_ERROR_EMPTY_STRING) {
        const char* err = cln_get_last_err();
        printf("test_layout failed\n  empty_str_r error code: %d\n  error message: %s\n", empty_str_r, err);
        ERROR_RETURN(buffer);
    }
    if(double_r < CLN_SUCCESS) {
        const char* err = cln_get_last_err();
        printf("test_layout failed\n  double_r error code: %d\n  error message: %s\n", double_r, err);
        ERROR_RETURN(buffer);
    }

    const size_t layout_len = 3;
    const cln_layout layout[] = {CLN_UINT8, CLN_STRING, CLN_DOUBLE};
    const int32_t set_layout_r = cln_set_layout(&buffer, layout, layout_len);
    if(set_layout_r < CLN_SUCCESS) {
        const char* err = cln_get_last_err();
        printf("test_layout failed\n  set_layout_r error code: %d\n  error message: %s\n", set_layout_r, err);
        ERROR_RETURN(buffer);
    }
    
    uint8_t* read_uint = NULL;
    char* read_empty_str = NULL;
    double* read_double = NULL;
    int32_t retrieve_items_r = cln_retrieve_items(&buffer, &read_uint, &read_empty_str, &read_double);
    if(retrieve_items_r < CLN_SUCCESS) {
        //TODO: change to this when retrieve_items will write to cln_last_error
        // const char* err = cln_get_last_err();
        // printf("test_layout1 failed\n  retrieve_items_r error code: %d\n  error message: %s\n", retrieve_items_r, err);
        // ERROR_RETURN(buffer);
        
        printf("test_layout failed\n  retrieve_items_r error code: %d\n\n", retrieve_items_r);
        ERROR_RETURN(buffer);
    }

    if(*read_uint != _uint) {
        printf("test_layout failed\n  *read_uint: %d\n  _uint: %d\n\n", *read_uint, _uint);
        ERROR_RETURN(buffer);
    }
    if(strcmp(read_empty_str, empty_str)) {
        printf("test_layout failed\n  read_empty_str doesn't match empty_str\n\n");
        ERROR_RETURN(buffer);
    }
    if(*read_double != _double) {
        printf("test_layout failed\n  *read_double: " __TEST_D_FORMAT "\n  _double: " __TEST_D_FORMAT "\n\n", *read_double, _double);
        ERROR_RETURN(buffer);
    }

    SUCCESS_RETURN(buffer);
}

// tests cln_read_buffer, cln_retrieve_items
int32_t test_read_buffer(void) {
#define __TEST_STR1 "frivolous\\,\\,!!!!"
#define __TEST_INT16 82
#define __TEST_STR2 "73"
    const char* cln_source = "frivolous\\,\\,!!!!,82,73";
    const size_t layout_len = 3;
    const cln_layout layout[] = {CLN_STRING, CLN_INT16, CLN_STRING};
    cln_buffer buffer = cln_read_buffer(cln_source);
    const int32_t set_layout_r = cln_set_layout(&buffer, layout, layout_len);
    if(set_layout_r < CLN_SUCCESS) {
        const char* err = cln_get_last_err();
        printf("test_read_buffer failed\n  set_layout_r error code: %d\n  error message: %s\n", set_layout_r, err);
        ERROR_RETURN(buffer);
    }

    char* read_str1 = NULL;
    int16_t* read_int = NULL;
    char* read_str2 = NULL;
    int32_t retrieve_items_r = cln_retrieve_items(&buffer, &read_str1, &read_int, &read_str2);
    if(retrieve_items_r < CLN_SUCCESS) {
        //TODO: change to this when retrieve_items will write to cln_last_error
        // const char* err = cln_get_last_err();
        // printf("test_layout1 failed\n  retrieve_items_r error code: %d\n  error message: %s\n", retrieve_items_r, err);
        // ERROR_RETURN(buffer);
        
        printf("test_read_buffer failed\n  retrieve_items_r error code: %d\n\n", retrieve_items_r);
        ERROR_RETURN(buffer);
    }

    if(strcmp(read_str1, __TEST_STR1)) {
        printf("test_readbuffer failed\n strings don't match\n  read_str1: %s\n  str1: %s\n\n", read_str1, __TEST_STR1);
        ERROR_RETURN(buffer);
    }
    if(*read_int != __TEST_INT16) {
        printf("test_readbuffer failed\n  *read_int: %d\n  _int: %d\n\n", *read_int, __TEST_INT16);
        ERROR_RETURN(buffer);
    }
    if(strcmp(read_str2, __TEST_STR2)) {
        printf("test_readbuffer failed\n strings don't match\n  read_str2: %s\n  str2: %s\n\n", read_str2, __TEST_STR2);
        ERROR_RETURN(buffer);
    }

    SUCCESS_RETURN(buffer);
}

int main(void) {
    uint32_t failed_count = 0;
    failed_count += test_add_buffer();
    failed_count += test_layout();
    failed_count += test_read_buffer();

    if(failed_count == 0) {
        printf("\033[32mAll tests were successful.\033[0m\n");
    } else {
        printf("\033[31m%d of the tests failed.\033[0m\n", failed_count);
        return 1;
    }

    return 0;
}
