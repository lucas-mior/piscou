#define CBASE_IMPLEMENT
#include "cbase.h"

#define PISCOU_REGEX_IMPLEMENT
#include "piscou_regex.h"

#if !PISCOU_REGEX_BACKEND_LIBC
#error Invalid pattern test requires the libc regex backend.
#endif

static PiscouRegex *invalid_regex = R("[");

int
main(void) {
    piscou_regex_match(invalid_regex, "sample", NULL, 0);
    return 0;
}
