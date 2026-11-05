/* minilib: a small library with two functions to fuzz, parse_ini and join_path. */
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

/* Join a directory and a file name with a '/': ("docs", "a.txt") -> "docs/a.txt".
 * dir and file are '\0'-terminated strings; out has room for outcap bytes.
 * Returns the length of the result, or -1 if it does not fit. */
int join_path(const char *dir, const char *file, char *out, size_t outcap);

#endif
