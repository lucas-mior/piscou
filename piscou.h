#ifndef PISCOU_H
#define PISCOU_H

#include <assert.h>
#include <errno.h>
#include <libgen.h>
#include <magic.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>

#ifndef INTEGERS
#define INTEGERS
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned long long ulonglong;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef size_t usize;
typedef ssize_t isize;
#endif

#if !defined(PISCOU_DEBUG)
  #define PISCOU_DEBUG 0
#endif
#if !defined(PISCOU_BENCHMARK)
  #define PISCOU_BENCHMARK 0
#endif

#define SNPRINTF(BUFFER, FORMAT, ...) \
    snprintf2(BUFFER, sizeof(BUFFER), FORMAT, __VA_ARGS__)
#define LENGTH(X) (int32)(sizeof(X) / sizeof(*X))

#define MATCH_SUBEXPRESSIONS(R, S, PMATCHES) \
    !regexec(&R.regex, S, LENGTH(PMATCHES), PMATCHES, 0)

#define MATCH_REGEX_SIMPLE(R, S) \
    !regexec(&R.regex, S, 0, NULL, 0)

typedef struct Regex {
    regex_t regex;
    char *string;
} Regex;

static void
compile_regex(Regex *regex) {
    if (regcomp(&regex->regex, regex->string, REG_EXTENDED)) {
        fprintf(stderr, "Could not compile regex %s.\n", regex->string);
        exit(EXIT_FAILURE);
    }
    return;
}

#endif
