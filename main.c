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

char *filename = NULL;
char *extras[9] = {NULL};
char config[256];
FILE *conf;
char *cargs[100] = {NULL};

void open_config(void) {
    printf("open_config()\n");
    char *cache = NULL;
    char *piscou = "piscou/piscou.conf";

    if (!(cache = getenv("XDG_CONFIG_HOME"))) {
        fprintf(stderr, "XDG_CONFIG_HOME needs to be set\n");
        exit(1);
    }
    printf("cache = %s\n", cache);

    snprintf(config, sizeof(config), "%s/%s", cache, piscou);
    config[255] = '\0';

     printf("config = %s\n", config);

    if (!(conf = fopen(config, "r"))) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }
}

void parse_args(void) {
    for (size_t i = 0; i < sizeof(cargs); i++) {
        if (cargs[i] == NULL)
            break;
        printf("cargs[%ld]=%s\n", i, cargs[i]);
        if (!strncmp(cargs[i], "%piscou-filename%", 100)) {
             cargs[i] = filename;
        }
    }
    return;
}

void iterate_conf(void) {
    char buf[256];
    char *pbuf;
    char *mime_conf = NULL;
    char *mime_file = NULL;
    char *comm = NULL;
    char *comp_conf = NULL;
    char *comp_file = NULL;

    regex_t r;
    int v;

    while (fgets(buf, sizeof(buf), conf)) {
        pbuf = buf;

        while ((*pbuf == ' ') || (*pbuf == '\t'))
            pbuf++;
        if ((*pbuf == '#') || (*pbuf == '\n'))
            continue;

        mime_conf = strtok(pbuf, " ");
        if (!strncmp(mime_conf, "fpath", 5)) {
            comp_conf = strtok(NULL, " \t");
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

        if (!regexec(&r, comp_file, 0, NULL, 0)) {
            printf("MATCH: %s!\n", comp_file);
        } else {
            printf("NO MATCH: %s != %s\n", comp_file, comp_conf);
            continue;
        }

        size_t i = 0;
        while ((comm = strtok(NULL, " \t\n"))) {
            printf("comm: %s\n", comm);
            cargs[i] = comm;
            i += 1;
        }
        parse_args();
        break;
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
    while (optind < argc) {
        extras[i] = argv[optind];
        optind += 1;
        i += 1;
    }

    preview();
    return 0;
}
