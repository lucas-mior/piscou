/* This file is part of piscou. */
/* Copyright (C) 2022 Lucas Mior */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>.*/

#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <regex.h>
#include <magic.h>
#include <limits.h>
#include <stdlib.h>

#include "util.h"
#include "piscou.h"

static char *filename = NULL;
static char *extras[10] = {NULL};

static void parse_args(char *cargs[]) {
    regex_t r_filename, r_extras;
    regmatch_t groups[10];

    regcomp(&r_filename, ".*(%piscou-filename%).*", REG_EXTENDED);
    regcomp(&r_extras, "%piscou-extra([0-9])%", REG_EXTENDED);

    for (size_t i = 0; i < 10; i++) {
        if (cargs[i] == NULL)
            break;

        if (!regexec(&r_filename, cargs[i], 10, groups, 0)) {
            cargs[i] = filename;
        }
        if (!regexec(&r_extras, cargs[i], 10, groups, 0)) {
            int num;
            char copy[strlen(cargs[i]) + 1];
            strcpy(copy, cargs[i]);
            copy[groups[1].rm_eo] = 0;
            num = atoi(copy + groups[1].rm_so);
            cargs[i] = extras[num];
        }
    }
    execvp(cargs[0], cargs);
    return;
}

static void preview(void) {
    char *mime_conf = NULL;
    char *mime_file = NULL;
    char *comp_conf = NULL;
    char *comp_file = NULL;

    regex_t r;
    int v;

    for (size_t i = 0; i < sizeof(rules); i++) {

        mime_conf = rules[i].mime;
        if (!strncmp(mime_conf, "fpath", 5)) {
            comp_conf = mime_conf + 6;
            while (*comp_conf == ' ')
                comp_conf++;
            comp_file = filename;
        } else {
            magic_t m;
            comp_conf = mime_conf;
            m = magic_open(MAGIC_MIME_TYPE);
            magic_load(m, NULL);
            mime_file = (char *) magic_file(m, filename);
            comp_file = mime_file;
        }

        v = regcomp(&r, comp_conf, REG_EXTENDED);
        if (v != 0) {
            fprintf(stderr, "Error creating regex for mime_conf %s\n", comp_conf);
            continue;
        }

        if (regexec(&r, comp_file, 0, NULL, 0) == REG_NOMATCH) {
            continue;
        } else {
            parse_args(rules[i].args);
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    int option;
    size_t i = 0;

    while ((option = getopt(argc, argv, "h")) != -1) {
        switch (option) {
        case 'h':
            usage(stdout);
        case '?':
            printf("unknown option: %c\n", optopt);
            break;
        }
    }
    if (argv[optind] != NULL)
        filename = argv[optind];
    else
        usage(stderr);

    optind += 1;
    while ((optind < argc) && i <= 9) {
        extras[i] = argv[optind];
        optind += 1;
        i += 1;
    }
    if((filename = realpath(filename, NULL)) == NULL) {
        perror("realpath() failed");
        exit(1);
    }

    preview();
    return 0;
}
