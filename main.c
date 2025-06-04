#include "cln.h"
#include <locale.h>
#include <stdio.h>
#include <string.h>
#define LAYOUT_LEN 4

int main(void) {
    cln_buffer buffer = cln_create_buffer(3);
    const char* s_str = "hello!";
    const int32_t s_int = 32;
    const double s_double = 17249812489.127491247819;
    const char* s_estr = "";
    cln_add_str(&buffer, s_str, strlen(s_str));
    cln_add_int(&buffer, s_int);
    cln_add_double(&buffer, s_double, "%f");
    cln_add_str(&buffer, s_estr, strlen(s_estr));
    const cln_layout x[LAYOUT_LEN] = {CLN_STRING, CLN_INT32, CLN_DOUBLE, CLN_STRING};
    cln_set_layout(&buffer, x, LAYOUT_LEN);
    printf("src str: |%s|\nsrc int: |%d|\nsrc double: |%f|\n", s_str, s_int, s_double);

    char* r_str = NULL;
    int32_t* r_int = NULL;
    double* r_double = NULL;
    char* r_estr = NULL;
    cln_alloc_retrieve_items(&buffer, &r_str, &r_int, &r_double, &r_estr);
    printf("res str: |%s|\nres int: |%d|\nres double: |%f|\n", r_str, *r_int, *r_double);
    printf("\nstr equal: %d\nint equal: %d\ndouble equal: %d\n", strcmp(s_str, r_str) == 0, s_int == *r_int, s_double == *r_double);
    printf("res empty str: |%s|\n", r_estr);

    free(r_str);
    free(r_int);
    free(r_double);
    free(r_estr);

    cln_free_buffer(&buffer);

    
    const char* sample = "55.3,very good";
    cln_buffer read_buffer = cln_read_buffer(sample);
    cln_set_layout(&read_buffer, (cln_layout[]){CLN_FLOAT, CLN_STRING}, 2);

    float* rr_float = NULL;
    char* rr_str = NULL;
    cln_alloc_retrieve_items(&read_buffer, &rr_float, &rr_str);
    printf("\n\nread buffer success: %d\n",
            *rr_float == 55.3f && strcmp(rr_str, "very good") == 0);

    free(rr_float);
    free(rr_str);
    
    cln_free_buffer(&read_buffer);
    return 0;
}
