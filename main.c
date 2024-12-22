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

#include "piscou.h"
#include "config.h"

typedef struct Array {
    char *array[MAX_ARGS];
    int32 len;
    int32 unused;
} Array;


static inline char *xmemdup(char *string, int32 n);
static inline int32 get_extra_number(char *, regmatch_t);
static inline void array_push(Array *, char *);
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
        if ((file_mime = magic_file(magic, filename)) == NULL)
            file_mime = "text/plain";
    } else {
        filename = argv[1];
        file_mime = "text/plain";
    }

    for (int32 i = 0; i < LENGTH(rules); i += 1) {
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

    for (int32 i = 0; command[i]; i += 1) {
        char *argument = command[i];
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
        if (MATCH_SUBEXPRESSIONS(regex_extras_more, argument, matches)) {
            char assembled[MAX_ARGUMENT_LENGTH] = {0};
            char *pointer = &assembled[0];
            int32 extra_length = 0;
            int32 final_length;
            strcpy(assembled, argument);
            do {
                char *argv_passed;
                int32 total_length;
                int32 start = matches[0].rm_so;
                int32 end = matches[0].rm_eo;
                int32 left = (int32) strlen(&pointer[end]) + 1;
                int32 extra_index = get_extra_number(pointer, matches[1]);

                if (extra_index >= argc) {
                    error("Extra argument %d not passed to piscou."
                          " Ignoring...\n", extra_index);
                    goto ignore;
                }

                argv_passed = argv[extra_index];
                extra_length = (int32) strlen(argv_passed);
                total_length = (int32) (pointer - &assembled[0])
                               + extra_length + left;
                if (total_length >= MAX_ARGUMENT_LENGTH) {
                    error("Too long argument. Max length is %d.\n",
                          MAX_ARGUMENT_LENGTH);
                    goto ignore;
                }

                memmove(&pointer[start + extra_length],
                        &pointer[end], (size_t) left);
                memcpy(&pointer[start],
                       argv_passed, (size_t) extra_length);
                pointer += (extra_length + start);
            } while (MATCH_SUBEXPRESSIONS(regex_extras_more, pointer, matches));

            final_length = (int32) (pointer - &assembled[0]);
            array_push(&args, xmemdup(assembled, final_length));
            continue;
        }
        array_push(&args, argument);
ignore:
        continue;
    }
#if PISCOU_DEBUG
    for (int32 i = 0; i < args.len + 1; i += 1)
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
xmemdup(char *string, int32 n) {
    char *p;

    if ((p = malloc((size_t)n)) == NULL) {
        error("Error allocating %zu bytes.\n", n);
        exit(EXIT_FAILURE);
    }

    memcpy(p, string, (size_t)n);
    return p;
}

int32
get_extra_number(char *string, regmatch_t pmatch) {
    char number_buffer[12] = {0};
    int32 start = pmatch.rm_so;
    int32 end = pmatch.rm_eo;
    int32 diff = end - start;
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
            for (int32 i = 0; i < LENGTH(notifiers); i += 1) {
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
