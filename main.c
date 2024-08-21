/* This file is part of piscou.
 * Copyright (C) 2024 Lucas Mior

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.

 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <libgen.h>
#include <magic.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>

#include "config.h"

typedef int32_t int32;
typedef uint32_t uint32;

#define LENGTH(X) (uint32) (sizeof (X) / sizeof (*X))

#define MATCH_SUBEXPRESSIONS(R, S, PMATCHES) \
    !regexec(&R.regex, S, LENGTH(PMATCHES), PMATCHES, 0)

#define MATCH_REGEX_SIMPLE(R, S) \
    !regexec(&R.regex, S, 0, NULL, 0)

typedef struct Array {
    char *array[MAX_ARGS];
    uint32 len;
    uint32 unused;
} Array;

typedef struct Regex {
    regex_t regex;
    char *string;
} Regex;

static inline char *xmemdup(char *string, uint32 n);
static inline int32 get_extra_number(char *, regmatch_t);
static inline void array_push(Array *, char *);
static inline void compile_regex(Regex *);
static inline void parse_command_run(char * const *, int32, char **);
static void error(char *, ...);
static void usage(FILE *) __attribute__((noreturn));

static char *filename;
static char *program;

int main(int argc, char **argv) {
    char buffer[PATH_MAX];
    magic_t magic;
    const char *file_mime = NULL;
    bool found = false;
    program = basename(argv[0]);

    if (argc <= 1)
        usage(stderr);

    if ((filename = realpath(argv[1], buffer))) {
        if ((magic = magic_open(MAGIC_MIME_TYPE)) == NULL) {
            error("Error in magic_open(MAGIC_MIME_TYPE):%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (magic_load(magic, NULL) != 0) {
            error("Error in magic_load(magic):%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if ((file_mime = magic_file(magic, filename)) == NULL) {
            magic_close(magic);
            file_mime = "text/plain";
        }
    } else {
        filename = argv[1];
        file_mime = "text/plain";
    }

    for (uint32 i = 0; i < LENGTH(rules); i += 1) {
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
        parse_command_run(rules[i].command, argc - 2, &argv[2]);
    }

    if (!found) {
        char error_message[512];
        int32 n = snprintf(error_message, sizeof (error_message),
                           "No previewer set for file:\n\n"
                           "%s:\n    %s\n", basename(argv[1]), file_mime);
        write(STDERR_FILENO, error_message, (size_t) n);
        write(STDOUT_FILENO, error_message, (size_t) n);
    }
    exit(EXIT_SUCCESS);
}

void
parse_command_run(char * const *command, int32 argc, char **argv) {
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

    for (uint32 i = 0; command[i]; i += 1) {
        char *argument = command[i];
        char *pointer = argument;
        regmatch_t matches[MAX_EXTRAS + 1];

        if (MATCH_REGEX_SIMPLE(regex_filename, argument)) {
            array_push(&args, filename);
            continue;
        }
        if (MATCH_SUBEXPRESSIONS(regex_extras, argument, matches)) {
            int32 extra_index = get_extra_number(argument, matches[1]);

            if (extra_index >= argc) {
                error("Extra argument %d not passed to piscou. Ignoring...\n",
                      extra_index);
                goto ignore;
            }
            array_push(&args, argv[extra_index]);
            continue;
        }
        if (MATCH_SUBEXPRESSIONS(regex_extras_more, pointer, matches)) {
            char copy[MAX_ARGUMENT_LENGTH] = {0};
            uint32 extra_len;
            uint32 arg_len;
            strcpy(copy, argument);
            do {
                char *extra;
                uint32 start = (uint32) matches[0].rm_so;
                uint32 end = (uint32) matches[0].rm_eo;
                uint32 diff = end - start;
                int32 extra_index = get_extra_number(copy, matches[1]);

                if (extra_index >= argc) {
                    error("Extra argument %d not passed to piscou."
                          " Ignoring...\n", extra_index);
                    goto ignore;
                }

                extra = argv[extra_index];
                extra_len = (uint32) strlen(extra);
                if (extra_len > diff) {
                    uint32 left = (uint32) strlen(copy + end);
                    if (left >= (MAX_ARGUMENT_LENGTH - (start + extra_len))) {
                        error("Too long argument. Max length is %d.\n",
                              MAX_ARGUMENT_LENGTH);
                        goto ignore;
                    }
                    memmove(copy + start + extra_len,
                            copy + end, (size_t) left + 1);
                    end = start + extra_len;
                }
                pointer = memmove(copy + start, extra, (size_t) extra_len);
                memmove(pointer + extra_len, copy + end, strlen(copy + end) + 1);
            } while (MATCH_SUBEXPRESSIONS(regex_extras_more, pointer, matches));

            arg_len = (uint32) (pointer + extra_len - copy);
            array_push(&args, xmemdup(copy, arg_len + 1));
            continue;
        }
        array_push(&args, argument);
ignore:
        continue;
    }
#if PISCOU_DEBUG
    for (uint32 i = 0; i < args.len + 1; i += 1)
        printf("args.array[%d] = %s\n", i, args.array[i]);
#else
    execvp(args.array[0], args.array);
    error("Error executing %s: %s\n", args.array[0], strerror(errno));
#endif
    return;
}

void
usage(FILE *stream) {
    fprintf(stream, "usage: piscou %%piscou-filename%%"
                    " [ %%piscou-extra0%% %%piscou-extra1%% ... ]\n");
    exit(stream != stdout);
}

char *
xmemdup(char *string, uint32 n) {
    char *p;

    if ((p = malloc(n)) == NULL) {
        error("Error allocating %zu bytes.\n", n);
        exit(EXIT_FAILURE);
    }

    memcpy(p, string, n);
    return p;
}

void
compile_regex(Regex *regex) {
    if (regcomp(&regex->regex, regex->string, REG_EXTENDED)) {
        error("Could not compile regex %s.\n", regex->string);
        exit(EXIT_FAILURE);
    }
    return;
}

int32
get_extra_number(char *string, regmatch_t pmatch) {
    char number_buffer[12] = {0};
    uint32 start = (uint32) pmatch.rm_so;
    uint32 end = (uint32) pmatch.rm_eo;
    uint32 diff = end - start;
    int32 number;

    memcpy(number_buffer, string + start, (size_t) diff);
    number = atoi(number_buffer);
    return number;
}

void
array_push(Array *array, char *string) {
    array->array[array->len] = string;
    array->len += 1;
    return;
}

void
error(char *format, ...) {
    int32 n;
    va_list args;
    char buffer[BUFSIZ];

    va_start(args, format);
    n = vsnprintf(buffer, sizeof (buffer) - 1, format, args);
    va_end(args);

    if (n < 0) {
        fprintf(stderr, "Error in vsnprintf()\n");
        exit(EXIT_FAILURE);
    }

    buffer[n] = '\0';
    write(STDERR_FILENO, buffer, (size_t) n);

#ifdef DEBUGGING
    switch (fork()) {
        char *notifiers[2] = { "dunstify", "notify-send" };
        case -1:
            fprintf(stderr, "Error forking: %s\n", strerror(errno));
            break;
        case 0:
            for (uint i = 0; i < LENGTH(notifiers); i += 1) {
                execlp(notifiers[i], notifiers[i], "-u", "critical", 
                                     program, buffer, NULL);
            }
            fprintf(stderr, "Error trying to exec dunstify.\n");
            break;
        default:
            break;
    }
    exit(EXIT_FAILURE);
#endif
}
