#include "cln.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    const char *a = "string1,,";
    const char *b = "string2,";
    const char *c = "string3";
    cln_buffer buffer = cln_create_buffer(3);
    int32_t codea = cln_add_str(&buffer, a, strlen(a));
    int32_t codeb = cln_add_str(&buffer, b, strlen(b));
    int32_t codec = cln_add_str(&buffer, c, strlen(c));
    cln_add_int(&buffer, -5);
    cln_add_uint(&buffer, 103849578998);
    cln_add_float(&buffer, 2178943.8349739f, "%.20g");
    cln_add_double(&buffer, 83295792178943.83832957949739,"%.50g");
    printf("original: %s\n", buffer.str);
    printf("code a: %d,     code b: %d     code c: %d\n", codea, codeb, codec);
    printf("capacity: %zu   size: %zu\n\n", buffer.capacity, buffer.size);

    cln_trim_capacity(&buffer);
    printf("trimmed:  %s\n", buffer.str);
    printf("capacity: %zu   size: %zu\n\n", buffer.capacity, buffer.size);
    printf("code a: %d,     code b: %d     code c: %d\n", codea, codeb, codec);

    const size_t layout_size = 5;
    const cln_layout layout[] = {CLN_UINT32, CLN_FLOAT, CLN_STRING, CLN_INT32, CLN_UINT8};
    cln_set_layout(&buffer, layout, layout_size);
    printf("first layout set working: %s\n", *buffer.layout == CLN_UINT32 ? "true" : "false");
    const cln_layout layout1[] = {CLN_STRING, CLN_FLOAT, CLN_STRING, CLN_INT32, CLN_UINT8};
    cln_set_layout(&buffer, layout1, layout_size);
    printf("second layout set working: %s\n", *buffer.layout == CLN_STRING ? "true" : "false");
    cln_free_buffer(&buffer);

    return 0;
}
