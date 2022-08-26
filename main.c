#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <regex.h>
#include <magic.h>

#include "util.h"
#include "piscou.h"

char *filename = NULL;
char *extras[10] = {NULL};
char config[256];
FILE *conf;

void open_config(void) {
    char *cache = NULL;
    char *piscou = "piscou/piscou.conf";

    if (!(cache = getenv("XDG_CONFIG_HOME"))) {
        fprintf(stderr, "XDG_CONFIG_HOME needs to be set\n");
        exit(1);
    }

    snprintf(config, sizeof(config), "%s/%s", cache, piscou);
    config[255] = '\0';

    if (!(conf = fopen(config, "r"))) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }
}

void parse_args(char *cargs[]) {
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
            char sourceCopy[strlen(cargs[i]) + 1];
            strcpy(sourceCopy, cargs[i]);
            sourceCopy[groups[1].rm_eo] = 0;
            int num = atoi(sourceCopy + groups[1].rm_so);
            cargs[i] = extras[num];
        }
    }
    execvp(cargs[0], cargs);
    return;
}

void iterate_conf(void) {
    char *mime_conf = NULL;
    char *mime_file = NULL;
    char *comp_conf = NULL;
    char *comp_file = NULL;

    regex_t r;
    int v;

    for (size_t i = 0; i < sizeof(commands); i++) { 

        mime_conf = commands[i].mime;
        if (!strncmp(mime_conf, "fpath", 5)) {
            comp_conf = strtok(mime_conf, " \t");
            comp_file = filename;
        } else {
            comp_conf = mime_conf;
            magic_t m;
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
            parse_args(commands[i].args);
            break;
        }
    }
    fclose(conf);
}

void preview(void) {
    open_config();
    iterate_conf();
}

int main(int argc, char *argv[]) {
    int option;
    while ((option = getopt(argc, argv, "h")) != -1) {
        switch (option) {
        case 'h':
            usage(stdout);
            break;
        case '?':
            printf("unknown option: %c\n", optopt);
            break;
        }
    }
    if (argv[optind] != NULL)
        filename = argv[optind];
    else
        usage(stderr);

    size_t i = 0;
    optind += 1;
    while ((optind < argc) && i <= 9) {
        extras[i] = argv[optind];
        optind += 1;
        i += 1;
    }

    preview();
    return 0;
}
