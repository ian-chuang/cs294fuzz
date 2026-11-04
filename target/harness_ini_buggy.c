/* Fuzz harness: copy stdin into a line buffer, then call parse_ini. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minilib.h"

int main(void) {
    static char buf[4096];
    size_t n = fread(buf, 1, sizeof buf, stdin);

    char text[64];
    memcpy(text, buf, n);                   /* copy the whole input */

    ini_config cfg;
    memset(&cfg, 0, sizeof cfg);
    parse_ini(text, n < sizeof text ? n : sizeof text, &cfg);
    ini_free(&cfg);
    return 0;
}
