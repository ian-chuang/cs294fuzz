/* Fuzz harness: feed stdin to parse_ini (a teammate's version). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minilib.h"

int main(void) {
    static char text[4096];
    /* fgets(s, size, stream) reads characters from stream into s until it has read
     * a newline (which it keeps), reached the end of the input, or read size - 1
     * characters. It adds a '\0' after them, and returns NULL if there was nothing to read. */
    if (fgets(text, sizeof text, stdin) == NULL) return 0;   /* read the input */
    size_t n = strlen(text);

    char *input = malloc(n ? n : 1);        /* exact-size copy, so overreads are caught */
    memcpy(input, text, n);

    ini_config cfg;
    memset(&cfg, 0, sizeof cfg);
    parse_ini(input, n, &cfg);
    ini_free(&cfg);
    free(input);
    return 0;
}
