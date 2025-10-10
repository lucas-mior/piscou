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

typedef int32_t int32;
typedef uint32_t uint32;

#define SNPRINTF(BUFFER, FORMAT, ...) \
    snprintf2(BUFFER, sizeof(BUFFER), FORMAT, __VA_ARGS__)
#define LENGTH(X) (int32)(sizeof(X) / sizeof(*X))

#define ARRAY_STRING(BUFFER, SEP, ARRAY, LENGTH) \
  _Generic((ARRAY), \
    int *: array_string(BUFFER, sizeof(BUFFER), SEP, "%d", ARRAY, LENGTH), \
    float *: array_string(BUFFER, sizeof(BUFFER), SEP, "%f", ARRAY, LENGTH), \
    double *: array_string(BUFFER, sizeof(BUFFER), SEP, "%f", ARRAY, LENGTH), \
    char **: array_string(BUFFER, sizeof(BUFFER), SEP, "%s", ARRAY, LENGTH) \
  )


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
