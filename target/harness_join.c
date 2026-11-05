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
    static char buf[4096];
    size_t n = fread(buf, 1, sizeof buf - 1, stdin);   /* the fuzzer's bytes: n of them */

    /* TODO 1: join_path wants two strings, dir and file. Get them from buf.
     *         Look at the seed (xxd seeds/join/basic.txt). memchr may help. */
    char *dir = buf;
    char *file = "";

    /* TODO 2: make sure dir and file are both '\0'-terminated strings.
     *         What if the input isn't in the format you expect? */

    /* TODO 3: choose how much room out has. */
    char out[4096];

    join_path(dir, file, out, sizeof out);
    (void)n;
    return 0;
}
