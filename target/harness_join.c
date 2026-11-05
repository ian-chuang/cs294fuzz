/* Fuzz harness for join_path. YOUR TURN: fill in the three TODOs.
 *
 *     int join_path(const char *dir, const char *file, char *out, size_t outcap);
 *     join_path("docs", "notes.txt", out, sizeof out)  puts "docs/notes.txt" in out
 *
 * The catch: the fuzzer gives you ONE block of bytes (buf, n bytes long), but
 * join_path wants TWO strings. The seed is "docs", a newline, then "notes.txt":
 *     xxd seeds/join/basic.txt
 *
 * Useful pieces of C:
 *     char *p = memchr(buf, '\n', n);   looks at the first n bytes of buf for a '\n';
 *                                       returns a pointer to the first one, or NULL
 *                                       if there is none (it doesn't stop at a 0 byte,
 *                                       unlike strchr)
 *     *p = '\0';                        ends a string at p; p + 1 starts the next one
 *     return 0;                         skips an input that isn't in your format
 *
 * Build and fuzz it:
 *     make build/fuzz_join
 *     afl-fuzz -i seeds/join -o runs/join -- build/fuzz_join
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minilib.h"

int main(void) {
    static char buf[4096];   /* static, so it starts as all 0 bytes */
    size_t n = fread(buf, 1, sizeof buf - 1, stdin);   /* the fuzzer's bytes: n of them, then a 0 byte */

    /* TODO 1: join_path wants two strings, dir and file. Get them from buf. */
    char *dir = buf;
    char *file = "";

    /* TODO 2: make sure dir and file are both '\0'-terminated strings.
     *         What should happen if the input has no newline at all? */

    /* TODO 3: choose how much room out has. Could join_path ever run out of room? */
    char out[4096];

    join_path(dir, file, out, sizeof out);
    (void)n;
    return 0;
}
