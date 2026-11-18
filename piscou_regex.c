#if !defined(PISCOU_REGEX_C)
#define PISCOU_REGEX_C

#if defined(__INCLUDE_LEVEL__) && (__INCLUDE_LEVEL__ == 0)
  #define CBASE_IMPLEMENT 1
#endif
#include "cbase.h"
#include "piscou_regex.h"

#if PISCOU_REGEX_BACKEND_LIBC

static PiscouRegex *piscou_regex_compiled_head;
static bool piscou_regex_cleanup_registered;

void
piscou_regex_cleanup(void) {
    PiscouRegex *regex = piscou_regex_compiled_head;

    piscou_regex_compiled_head = NULL;
    while (regex != NULL) {
        PiscouRegex *next = regex->next_compiled;
        regfree(&regex->compiled);
        regex->initialized = false;
        regex->next_compiled = NULL;
        regex = next;
    }
    return;
}

bool
piscou_regex_match(PiscouRegex *regex, char *string,
                   PiscouRegexMatch *matches, int32 matches_len) {
    int32 status;
    int32 nmatch;

    if (regex == NULL) {
        return false;
    }
    if (regex->pattern == NULL || string == NULL || matches_len < 0) {
        error("Invalid POSIX regex match arguments.\n");
        fatal(EXIT_FAILURE);
    }

    if (!regex->initialized) {
        status = regcomp(&regex->compiled, regex->pattern, REG_EXTENDED);
        if (status != 0) {
            char message[256];
            regerror(status, &regex->compiled, message, SIZEOF(message));
            error("Invalid POSIX regex '%s': %s\n", regex->pattern, message);
            fatal(EXIT_FAILURE);
        }
        if (!piscou_regex_cleanup_registered) {
            if (atexit(piscou_regex_cleanup) != 0) {
                regfree(&regex->compiled);
                error("Could not register POSIX regex cleanup.\n");
                fatal(EXIT_FAILURE);
            }
            piscou_regex_cleanup_registered = true;
        }
        regex->next_compiled = piscou_regex_compiled_head;
        piscou_regex_compiled_head = regex;
        regex->initialized = true;
    }

    nmatch = matches != NULL ? matches_len : 0;
    if ((regex->flags & META_RE_NOSUB) && nmatch > 1) {
        nmatch = 1;
    }
    if (matches) {
        for (int32 i = 0; i < matches_len; i += 1) {
            matches[i].rm_so = -1;
            matches[i].rm_eo = -1;
        }
    }
    status = regexec(&regex->compiled, string, (size_t)nmatch, matches, 0);
    if (status == REG_NOMATCH) {
        return false;
    }
    if (status != 0) {
        char message[256];
        regerror(status, &regex->compiled, message, SIZEOF(message));
        error("POSIX regex match failed for '%s': %s\n",
              regex->pattern, message);
        fatal(EXIT_FAILURE);
    }
    return true;
}

#endif /* PISCOU_REGEX_BACKEND_LIBC */

#if !defined(TESTING_piscou_regex)
  #if defined(__INCLUDE_LEVEL__) && (__INCLUDE_LEVEL__ == 0)
    #define TESTING_piscou_regex 1
  #else
    #define TESTING_piscou_regex 0
  #endif
#endif

#if TESTING_piscou_regex && PISCOU_REGEX_BACKEND_LIBC
static PiscouRegex *digits = R("^id-([0-9]+)$");
static PiscouRegex *anywhere = R("#piscou-([0-9])#");
static PiscouRegex *no_subs = R("(a)(b)", META_RE_NOSUB);

int
main(void) {
    PiscouRegexMatch matches[3];

    ASSERT(piscou_regex_match(digits, "id-23", matches, LENGTH(matches)));
    ASSERT_EQ(matches[0].rm_so, 0);
    ASSERT_EQ(matches[0].rm_eo, 5);
    ASSERT_EQ(matches[1].rm_so, 3);
    ASSERT_EQ(matches[1].rm_eo, 5);
    ASSERT_EQ(matches[2].rm_so, -1);
    ASSERT(!piscou_regex_match(digits, "id-x", matches, LENGTH(matches)));
    ASSERT_EQ(matches[0].rm_so, -1);

    ASSERT(piscou_regex_match(anywhere, "x#piscou-7#y",
                              matches, LENGTH(matches)));
    ASSERT_EQ(matches[0].rm_so, 1);
    ASSERT_EQ(matches[0].rm_eo, 11);
    ASSERT_EQ(matches[1].rm_so, 9);
    ASSERT_EQ(matches[1].rm_eo, 10);

    ASSERT(piscou_regex_match(no_subs, "zabz", matches, LENGTH(matches)));
    ASSERT_EQ(matches[0].rm_so, 1);
    ASSERT_EQ(matches[0].rm_eo, 3);
    ASSERT_EQ(matches[1].rm_so, -1);
    ASSERT(!piscou_regex_match(NULL, "text", NULL, 0));

    piscou_regex_cleanup();
    ASSERT(!digits->initialized);
    ASSERT(piscou_regex_match(digits, "id-1", matches, LENGTH(matches)));
    piscou_regex_cleanup();
    exit(EXIT_SUCCESS);
}
#endif /* TESTING_piscou_regex */

#endif /* PISCOU_REGEX_C */
