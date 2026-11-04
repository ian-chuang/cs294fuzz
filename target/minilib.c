#include "minilib.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------- INI parser ---------- */

/* A config file looks like this:
 *
 *     [server]          <- a section header: sets the "current section"
 *     host=localhost    <- a key=value line, stored under the current section
 *     # a comment       <- ignored, as are blank lines
 *
 * parse_ini() reads it one line at a time and fills in an ini_config.
 */

/* Return a heap copy of the first n characters of s, with a '\0' added.
 * (s is not necessarily '\0'-terminated, which is why we pass the length.) */
static char *copy_n(const char *s, size_t n) {
    char *p = malloc(n + 1);    /* n characters + 1 byte for the '\0' */
    memcpy(p, s, n);
    p[n] = '\0';
    return p;
}

/* How deeply a section name is nested: "a" is 1, "a.b" is 2, "a.b.c" is 3. */
static int section_depth(const char *name) {
    int depth = 1;
    for (; *name; name++)       /* stop at the '\0' that ends the string */
        if (*name == '.') depth++;
    return depth;
}

/* Free everything parse_ini() allocated for the entries in cfg. */
void ini_free(ini_config *cfg) {
    for (int i = 0; i < cfg->count; i++) {
        free(cfg->entries[i].section);
        free(cfg->entries[i].key);
        free(cfg->entries[i].value);
    }
    cfg->count = 0;
}

/* text is NOT '\0'-terminated: only the first len bytes are valid, so every
 * loop below must check against len before reading text[...]. */
int parse_ini(const char *text, size_t len, ini_config *cfg) {
    size_t pos = 0;         /* where the current line starts in text */
    char *section = NULL;   /* name of the current [section], heap-allocated */
    cfg->count = 0;

    while (pos < len) {
        /* Find the end of this line: eol is the index of the '\n' (or len). */
        size_t eol = pos;
        while (eol < len && text[eol] != '\n') eol++;
        const char *line = text + pos;      /* start of the line */
        size_t llen = eol - pos;            /* length of the line, without '\n' */
        size_t next = (eol < len) ? eol + 1 : len;  /* start of the next line */

        if (llen == 0 || line[0] == '#') {          /* blank line or comment */
            pos = next;
            continue;
        }

        if (line[0] == '[') {                       /* [section] header */
            /* Find the closing ']'. */
            size_t close = 1;
            while (close < llen && line[close] != ']') close++;
            if (close >= llen) goto fail;           /* missing ']' */
            size_t nlen = close - 1;                /* length of the name inside [] */
            if (nlen == 0 || nlen >= INI_NAME_MAX) goto fail;
            free(section);                          /* forget the previous section */
            section = copy_n(line + 1, nlen);
            /* Sanity check: nesting like [a.b.c] must not exceed the limit. */
            assert(section_depth(section) <= INI_MAX_DEPTH);
            pos = next;
            continue;
        }

        /* Otherwise the line must be key=value. Find the '='. */
        size_t eq = 0;
        while (eq < llen && line[eq] != '=') eq++;
        if (eq == llen) goto fail;                  /* no '=' on this line */
        size_t klen = eq;                           /* the key is everything before '=' */
        if (klen == 0 || klen > INI_KEY_MAX) goto fail;
        if (cfg->count >= INI_MAX_ENTRIES) goto fail;   /* table is full */

        /* Copy the key into its own heap buffer of INI_KEY_MAX bytes. */
        char *key = malloc(INI_KEY_MAX);
        memcpy(key, line, klen);
        key[klen] = '\0';

        /* The value is everything after '='. It is built up in val[] (a small
         * buffer on the stack) because a trailing backslash means "this value
         * continues on the next line", so it can span several lines. */
        char val[INI_VAL_MAX];
        size_t vlen = 0;                    /* how much of val[] is filled */
        const char *vp = line + eq + 1;     /* start of the current piece of value */
        size_t vl = llen - eq - 1;          /* length of that piece */
        for (;;) {
            int cont = (vl > 0 && vp[vl - 1] == '\\');  /* ends with a backslash? */
            size_t take = cont ? vl - 1 : vl;           /* drop the backslash itself */
            if (vlen + take >= INI_VAL_MAX) { free(key); goto fail; }  /* too long */
            memcpy(val + vlen, vp, take);
            vlen += take;
            if (!cont) break;               /* no backslash: the value is complete */
            if (next >= len) continue;      /* no next line to join */
            /* Move on to the next line and treat it as the next piece. */
            pos = next;
            eol = pos;
            while (eol < len && text[eol] != '\n') eol++;
            vp = text + pos;
            vl = eol - pos;
            next = (eol < len) ? eol + 1 : len;
        }

        /* Store the finished entry (section, key, value) in cfg. */
        ini_entry *e = &cfg->entries[cfg->count++];
        e->section = copy_n(section, strlen(section));
        e->key = key;
        e->value = copy_n(val, vlen);
        pos = next;
    }
    free(section);
    return 0;

fail:       /* every error above jumps here: clean up and report "invalid" */
    free(section);
    return -1;
}

/* ---------- URL decoding ---------- */

/* Value of one hex digit: '7' -> 7, 'a' or 'A' -> 10. -1 if c is not a hex digit. */
static int hexval(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/* URL encoding: "%2C" means the byte 0x2C (','), and '+' means a space.
 * in has len bytes (not necessarily '\0'-terminated); out has room for outcap. */
int url_decode(const char *in, size_t len, char *out, size_t outcap) {
    size_t o = 0;                       /* how many bytes written to out so far */
    for (size_t i = 0; i < len; i++) {
        if (o + 1 >= outcap) return -1; /* keep room for the final '\0' */
        if (in[i] == '%') {
            /* "%XY": the next two characters are hex digits of one byte. */
            int hi = hexval(in[i + 1]);
            int lo = hexval(in[i + 2]);
            if (hi < 0 || lo < 0) return -1;
            out[o++] = (char)(hi * 16 + lo);
            i += 2;                     /* skip the two hex digits */
        } else if (in[i] == '+') {
            out[o++] = ' ';
        } else {
            out[o++] = in[i];
        }
    }
    out[o] = '\0';
    return (int)o;
}

/* ---------- the rest of the library ---------- */

/* Adler-32 checksum: two running sums, a (sum of bytes) and b (sum of the
 * values of a), both kept below 65521. They are packed into one 32-bit
 * number, b in the top 16 bits and a in the bottom 16. */
unsigned checksum(const unsigned char *data, size_t len) {
    unsigned a = 1, b = 0;
    for (size_t i = 0; i < len; i++) {
        a = (a + data[i]) % 65521;
        b = (b + a) % 65521;
    }
    return (b << 16) | a;
}

/* 1234 -> "$12.34", -5 -> "-$0.05". Returns what snprintf returns: the
 * length the text would have, and snprintf never writes more than cap bytes. */
int format_cents(long cents, char *buf, size_t cap) {
    int neg = cents < 0;
    if (neg) cents = -cents;            /* work with a positive number */
    /* cents / 100 = whole dollars, cents % 100 = the two-digit remainder */
    return snprintf(buf, cap, "%s$%ld.%02ld", neg ? "-" : "", cents / 100, cents % 100);
}

/* A fixed table of (name, id) pairs. */
static const struct { const char *name; int id; } USERS[] = {
    {"admin", 1}, {"alice", 2}, {"bob", 3}, {"carol", 4}, {"dave", 5}, {"erin", 6},
};

/* Linear search through the table. name must be a '\0'-terminated string. */
int lookup_user(const char *name) {
    /* sizeof USERS / sizeof USERS[0] = number of entries in the table */
    for (size_t i = 0; i < sizeof USERS / sizeof USERS[0]; i++)
        if (strcmp(USERS[i].name, name) == 0) return USERS[i].id;
    return -1;
}
