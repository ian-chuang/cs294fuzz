/* minilib: a small utility library used by an imaginary web app. */
#ifndef MINILIB_H
#define MINILIB_H

#include <stddef.h>

/* Limits for the INI parser. Anything bigger than these is rejected. */
#define INI_MAX_ENTRIES 16
#define INI_NAME_MAX    16   /* longest section name */
#define INI_KEY_MAX      8   /* longest key */
#define INI_VAL_MAX     16   /* longest value */
#define INI_MAX_DEPTH    3   /* sections may be nested like [a.b.c] */

/* One key=value line. All three strings are separate heap allocations
 * (freed by ini_free). */
typedef struct {
    char *section;   /* the [section] it appeared under */
    char *key;
    char *value;
} ini_entry;

/* The parsed file: a fixed-size table of entries, of which the first
 * `count` are filled in. */
typedef struct {
    ini_entry entries[INI_MAX_ENTRIES];
    int count;
} ini_config;

/* Parse "[section]" headers and "key=value" lines from untrusted text.
 * text is not '\0'-terminated; only the first len bytes are valid.
 * Returns 0 on success, -1 if the text is not valid. The caller must call
 * ini_free(cfg) afterwards to release the strings stored in cfg. */
int parse_ini(const char *text, size_t len, ini_config *cfg);
void ini_free(ini_config *cfg);

/* Decode a URL-encoded string ("a%20b+c") into out. Returns the decoded
 * length, or -1 if the input is invalid or does not fit. */
int url_decode(const char *in, size_t len, char *out, size_t outcap);

/* A simple Fletcher-style checksum of a byte buffer. */
unsigned checksum(const unsigned char *data, size_t len);

/* Format an amount in cents as "$12.34" (or "-$12.34"). */
int format_cents(long cents, char *buf, size_t cap);

/* Look up a user id by name in the built-in user table. -1 if unknown. */
int lookup_user(const char *name);

#endif
