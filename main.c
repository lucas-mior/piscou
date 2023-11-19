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

#define MATCH_SUBEXPRESSIONS(R, S, PMATCHES) \
    !regexec(&R.regex, S, LENGTH(PMATCHES), PMATCHES, 0)

#define MATCH_REGEX_SIMPLE(R, S) \
    !regexec(&R.regex, S, 0, NULL, 0)

typedef struct Array {
    char *array[MAX_ARGS];
    int len;
    int unused;
} Array;

typedef struct Regex {
    regex_t regex;
    char *string;
} Regex;

static char *xstrdup(char *);
static int get_extra_number(char *, regmatch_t);
static int get_mime(char *, char *);
static void array_push(Array *, char *);
static void compile_regex(Regex *);
static void parse_command_run(char * const *, int, char **);
static void usage(FILE *) __attribute__((noreturn));

static char *filename;

int main(int argc, char **argv) {
    char buffer[256];
    char *file_mime = NULL;
    bool found = false;

    if (argc <= 1)
        usage(stderr);

    if ((filename = realpath(argv[1], NULL))) {
        if (get_mime(buffer, filename) < 0)
            file_mime = "text/plain";
        else
            file_mime = buffer;
    } else {
        filename = argv[1];
        file_mime = "text/plain";
    }

    for (int i = 0; i < LENGTH(rules); i += 1) {
        char *mime = rules[i].match[0];
        char *path = rules[i].match[1];

        if ((mime == NULL) && (path == NULL))
            continue;

        if (mime) {
            Regex regex_config;
            regex_config.string = mime;
            compile_regex(&regex_config);
            if (!MATCH_REGEX_SIMPLE(regex_config, file_mime))
                continue;
        }
        if (path) {
            Regex regex_config;
            regex_config.string = path;
            compile_regex(&regex_config);
            if (!MATCH_REGEX_SIMPLE(regex_config, filename))
                continue;
        }

        found = true;
        parse_command_run(rules[i].command, argc, argv);
    }

    if (!found) {
        printf("No previewer set for file: ");
        printf("%s: %s", filename, file_mime);
    }
    exit(EXIT_SUCCESS);
}

void parse_command_run(char * const *command, int argc, char **argv) {
    Array args = {0};
    Regex regex_filename;
    Regex regex_extras;
    Regex regex_extras_more;

    regex_filename.string = REGEX_FILENAME;
    regex_extras.string = REGEX_EXTRAS;
    regex_extras_more.string = REGEX_EXTRAS_MORE;

    compile_regex(&regex_filename);
    compile_regex(&regex_extras);
    compile_regex(&regex_extras_more);

    for (int i = 0; command[i]; i += 1) {
        char *argument = command[i];
        regmatch_t pmatch[MAX_EXTRAS + 1];

        if (MATCH_REGEX_SIMPLE(regex_filename, argument)) {
            array_push(&args, filename);
            continue;
        }
        if (MATCH_SUBEXPRESSIONS(regex_extras, argument, pmatch)) {
            int extra_index = get_extra_number(argument, pmatch[1]);

            if ((extra_index + 2) >= argc) {
                fprintf(stderr, "Extra argument %d not passed to piscou."
                                " Ignoring...\n", extra_index);
                goto ignore;
            }
            array_push(&args, argv[extra_index + 2]);
            continue;
        }
        if (MATCH_SUBEXPRESSIONS(regex_extras_more, argument, pmatch)) {
            char *pos;
            char copy[MAX_ARGUMENT_LENGTH] = {0};
            strcpy(copy, argument);
            do {
                char *extra;
                int extra_len;
                int start = pmatch[0].rm_so;
                int end = pmatch[0].rm_eo;
                int diff = end - start;
                int extra_index = get_extra_number(copy, pmatch[1]);

                if ((extra_index + 2) >= argc) {
                    fprintf(stderr, "Extra argument %d not passed to piscou."
                                    " Ignoring...\n", extra_index);
                    goto ignore;
                }

                extra = argv[extra_index + 2];
                extra_len = (int) strlen(extra);
                if (extra_len > diff) {
                    int left = (int) strlen(copy + end);
                    if (left >= (MAX_ARGUMENT_LENGTH - (start + extra_len))) {
                        fprintf(stderr, "Too long argument."
                                        "Max length is %d.\n",
                                         MAX_ARGUMENT_LENGTH);
                        goto ignore;
                    }
                    memmove(copy + start + extra_len,
                            copy + end, (size_t) left + 1);
                    end = start + extra_len;
                }
                pos = memmove(copy + start, extra, (size_t) extra_len);
                memmove(pos + extra_len, copy + end, strlen(copy + end) + 1);
            } while (MATCH_SUBEXPRESSIONS(regex_extras_more, pos, pmatch));

            array_push(&args, xstrdup(copy));
ignore:
            continue;
        }
        array_push(&args, argument);
    }
#ifdef PISCOU_DEBUG
    for (int i = 0; i < args.len + 1; i += 1)
        printf("args.array[%d] = %s\n", i, args.array[i]);
#else
    execvp(args.array[0], args.array);
    fprintf(stderr, "Error executing %s: %s\n",
                    args.array[0], strerror(errno));
#endif
    return;
}

int get_mime(char *buffer, char *file) {
    do {
        magic_t magic;
        const char *mime;
        if ((magic = magic_open(MAGIC_MIME_TYPE)) == NULL) {
            break;
        }
        if (magic_load(magic, NULL) != 0) {
            magic_close(magic);
            break;
        }
        if ((mime = magic_file(magic, file)) == NULL) {
            magic_close(magic);
            break;
        }
        strcpy(buffer, mime);
        return 0;
    } while (0);

    return -1;
}

void usage(FILE *stream) {
    fprintf(stream, "usage: piscou %%piscou-filename%%"
                    " [ %%piscou-extra0%% %%piscou-extra1%% ... ]\n");
    exit(stream != stdout);
}

char *xstrdup(char *string) {
    char *p;
    size_t size;

    size = strlen(string) + 1;
    if ((p = malloc(size)) == NULL) {
        fprintf(stderr, "Error allocating %zu bytes.\n", size);
        exit(EXIT_FAILURE);
    }

    memcpy(p, string, size);
    return p;
}

void compile_regex(Regex *regex) {
    if (regcomp(&regex->regex, regex->string, REG_EXTENDED)) {
        fprintf(stderr, "Could not compile regex %s.\n", regex->string);
        exit(EXIT_FAILURE);
    }
    return;
}

int get_extra_number(char *string, regmatch_t pmatch) {
    char number_buffer[12] = {0};
    int start = pmatch.rm_so;
    int end = pmatch.rm_eo;
    int diff = end - start;
    int number;

    memcpy(number_buffer, string + start, (size_t) diff);
    number = atoi(number_buffer);
    return number;
}

void array_push(Array *array, char *string) {
    array->array[array->len] = string;
    array->len += 1;
    return;
}
