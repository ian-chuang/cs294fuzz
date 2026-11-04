/* Fuzz harness: feed stdin to parse_ini. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minilib.h"

int main(void) {
    static char buf[4096];
    size_t n = fread(buf, 1, sizeof buf, stdin);

    char *input = malloc(n ? n : 1);        /* exact-size copy, so overreads are caught */
    memcpy(input, buf, n);

    ini_config cfg;
    memset(&cfg, 0, sizeof cfg);
    parse_ini(input, n, &cfg);
    ini_free(&cfg);
    free(input);
    return 0;
}
