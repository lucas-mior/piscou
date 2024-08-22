#ifndef PISCOU_H
#define PISCOU_H

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

typedef int32_t int32;
typedef uint32_t uint32;

#define LENGTH(X) (uint32) (sizeof (X) / sizeof (*X))

#define MATCH_SUBEXPRESSIONS(R, S, PMATCHES) \
    !regexec(&R.regex, S, LENGTH(PMATCHES), PMATCHES, 0)

#define MATCH_REGEX_SIMPLE(R, S) \
    !regexec(&R.regex, S, 0, NULL, 0)

typedef struct Regex {
    regex_t regex;
    char *string;
} Regex;

#endif
