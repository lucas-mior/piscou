#if !defined(PISCOU_H)
#define PISCOU_H

#include <magic.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <libgen.h>
#include <errno.h>

#if !defined(DEBUGGING)
  #define DEBUGGING 0
#endif
#if !defined(PISCOU_BENCHMARK)
  #define PISCOU_BENCHMARK 0
#endif

#define MATCH_SUBEXPRESSIONS(R, S, PMATCHES) \
    (!regexec(R, S, LENGTH(PMATCHES), PMATCHES, 0))

#define MATCH_REGEX_SIMPLE(R, S) \
    (!regexec(R, S, 0, NULL, 0))

#endif
