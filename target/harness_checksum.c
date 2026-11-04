/* Fuzz harness: feed stdin to checksum. */
#include <stdio.h>
#include "minilib.h"

int main(void) {
    static unsigned char buf[4096];
    size_t n = fread(buf, 1, sizeof buf, stdin);
    volatile unsigned sum = checksum(buf, n);
    (void)sum;
    return 0;
}
