#include <regex.h>
#include <stdio.h>
#include <stdint.h>

#include "config.h"

typedef uint32_t uint32;
#define LENGTH(X) (uint32) (sizeof (X) / sizeof (*X))

typedef struct Regex {
    regex_t regex;
    char *string;
} Regex;

int
main(void) {
    printf("precomp.c\n");
    Regex regexes[LENGTH(rules)];

    for (uint32 i = 0; i < LENGTH(rules); i += 1) {
        Rule rule = rules[i];
        printf("[%d] = %s\n", i, rules[i].match[0]);
        regexes[i].string = rule.match[0];
    }

    exit(EXIT_SUCCESS);
}
