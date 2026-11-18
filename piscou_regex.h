#if !defined(PISCOU_REGEX_H)
#define PISCOU_REGEX_H

#include "cbase.h"

#if !defined(PISCOU_REGEX_BACKEND_LIBC)
#define PISCOU_REGEX_BACKEND_LIBC 0
#endif

#if PISCOU_REGEX_BACKEND_LIBC
#include <regex.h>

enum PiscouRegexFlags {
    META_RE_NONE = 0,
    META_RE_YESSUB = 1,
    META_RE_NOSUB = 2,
};

/* Compiled patterns stay registered for cleanup; descriptors must outlive
   their use and remain valid through process exit. */
typedef struct PiscouRegex {
    char *pattern;
    regex_t compiled;
    enum PiscouRegexFlags flags;
    bool initialized;
    struct PiscouRegex *next_compiled;
} PiscouRegex;

typedef regmatch_t PiscouRegexMatch;

#define PISCOU_R_SELECT(_1, _2, NAME, ...) NAME
#define PISCOU_R_1(STR) (&(PiscouRegex){.pattern = (STR)})
#define PISCOU_R_2(STR, FLAGS)                                  \
    (&(PiscouRegex){                                            \
        .pattern = (STR),                                       \
        .flags = (enum PiscouRegexFlags)(FLAGS),                \
    })
#define R(...) \
    PISCOU_R_SELECT(__VA_ARGS__, PISCOU_R_2, PISCOU_R_1, 0)(__VA_ARGS__)
#else
#include "meta_regex/src/meta_regex.h"

typedef MetaRegex PiscouRegex;
typedef MetaRegexMatch PiscouRegexMatch;
#endif

bool piscou_regex_match(PiscouRegex *, char *, PiscouRegexMatch *, int32);

#if PISCOU_REGEX_BACKEND_LIBC
void piscou_regex_cleanup(void);
#endif

#if defined(PISCOU_REGEX_IMPLEMENT)
  #if PISCOU_REGEX_BACKEND_LIBC
    #include "piscou_regex.c"
  #else
#include "meta_regex/src/meta_match.c"

#define PISCOU_META_MATCHERS                                          \
    (enum Matcher)                                                    \
    (MATCHER_BTNFA | MATCHER_TDFA | MATCHER_LAZY_DFA                  \
     | MATCHER_STATIC_DFA)

bool
piscou_regex_match(PiscouRegex *regex, char *string,
                   PiscouRegexMatch *matches, int32 matches_len) {
    return meta_regex_match(regex, (uint8 *)string, strlen32(string),
                            matches, matches_len, PISCOU_META_MATCHERS) == 0;
}
  #endif
#endif

#endif
