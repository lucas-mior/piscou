#include <regex.h>
#include <stdio.h>
#include <stdint.h>

#include "config.h"
#include "piscou.h"

int
main(void) {
    printf("precomp.c\n");
    Regex regexes[LENGTH(rules)];

    for (uint32 i = 0; i < LENGTH(rules); i += 1) {
        Rule rule = rules[i];
        Regex *regex = &regexes[i];
        printf("[%d] = %s\n", i, rules[i].match[0]);
        regex->string = rule.match[0];
        if (regex->string)
            compile_regex(regex);
    }

    exit(EXIT_SUCCESS);
}
