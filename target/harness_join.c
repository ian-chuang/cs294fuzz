/* Fuzz harness for join_path. YOUR TURN: fill in the three TODOs.
 *
 *     int join_path(const char *dir, const char *file, char *out, size_t outcap);
 *     join_path("docs", "notes.txt", out, sizeof out)  puts "docs/notes.txt" in out
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

    /* TODO 1: join_path wants two strings, dir and file, but the fuzzer gives you
     *         one block of bytes. Split buf in two.
     *         Look at the seed first: xxd seeds/join/basic.txt  ("docs", a newline, "notes.txt")
     *         Find the newline with
     *             char *p = memchr(buf, '\n', n);
     *         memchr looks at the first n bytes of buf for the byte '\n' and returns
     *         a pointer to the first one, or NULL if there is none. (Unlike strchr,
     *         it doesn't stop early at a 0 byte.) dir starts at buf, file just after p. */
    char *dir = buf;
    char *file = "";

    /* TODO 2: make sure dir and file are both '\0'-terminated strings.
     *         *p = '\0';  ends dir at the newline. file already ends at the 0 byte after the input.
     *         If memchr returned NULL, the input has no newline: it isn't in your format.
     *         Skip it with  return 0;  (don't crash: a crash here is your bug, not join_path's). */

    /* TODO 3: choose how much room out has.
     *         join_path checks the names fit in outcap. With 4096 bytes and at most
     *         4095 bytes of input, could they ever not fit? What size would test that check? */
    char out[4096];

    join_path(dir, file, out, sizeof out);
    (void)n;
    return 0;
}
