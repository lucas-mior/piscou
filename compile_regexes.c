#include <errno.h>
#include <limits.h>
#include <magic.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "config.h"

#define LENGTH(X) (int) (sizeof (X) / sizeof (*X))

int main(int argc, char **argv) {
    for (int i = 0; i < LENGTH(rules); i += 1) {
        Rule *rule = &rules[i];
        char *mime = rules[i].match[0];
        char *path = rules[i].match[1];

        if ((mime == NULL) && (path == NULL))
            continue;

        if (mime) {
            if (regcomp(&rule->regex_mime, mime, REG_EXTENDED)) {
                fprintf(stderr, "Could not compile regex %s.\n", mime);
                exit(EXIT_FAILURE);
            }
            printf("Rule[%d] =\n", i);
            char *byte = (char *) &(rule->regex_mime);
            for (int j = 0; j < sizeof (rule->regex_mime); j += 1) {
                printf("%x ", i, byte[j]);
            }
            continue;
        }
        if (path) {
            if (regcomp(&rule->regex_path, path, REG_EXTENDED)) {
                fprintf(stderr, "Could not compile regex %s.\n", path);
                exit(EXIT_FAILURE);
            }
        }
    }
}
