/*
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
#include "util.c"

typedef struct Array {
    char arena[MAX_EXTRAS*MAX_ARGUMENT_LENGTH];
    char *array[MAX_ARGS];
    char *arena_pos;
    int32 len;
    int32 unused;
} Array;

static inline int32 get_extra_number(char *, regmatch_t);
static inline void array_push(Array *, char *, int32);
static inline void parse_command_run(char *const *, int32, char **);
static void usage(FILE *) __attribute__((noreturn));

static char *filename;
static char *program;
static Regex regex_filename;
static Regex regex_extras;
static Regex regex_extras_more;

int
main(int argc, char **argv) {
    char buffer[PATH_MAX];
    magic_t magic;
    const char *file_mime = NULL;
    bool found = false;
    program = basename(argv[0]);

    if (argc <= 1) {
        usage(stderr);
    }

    regex_filename.string = REGEX_FILENAME;
    regex_extras.string = REGEX_EXTRAS;
    regex_extras_more.string = REGEX_EXTRAS_MORE;

    compile_regex(&regex_filename);
    compile_regex(&regex_extras);
    compile_regex(&regex_extras_more);

    if ((filename = realpath(argv[1], buffer))) {
        if ((magic = magic_open(MAGIC_MIME_TYPE)) == NULL) {
            error("Error in magic_open(MAGIC_MIME_TYPE): %s.\n",
                  strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (magic_load(magic, NULL) != 0) {
            error("Error in magic_load(magic): %s.\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if ((file_mime = magic_file(magic, filename)) == NULL) {
            file_mime = "text/plain";
        }
    } else {
        filename = argv[1];
        file_mime = "text/plain";
    }

    for (int32 i = 0; i < LENGTH(rules); i += 1) {
        char *mime = rules[i].match[0];
        char *path = rules[i].match[1];

        if ((mime == NULL) && (path == NULL)) {
            continue;
        }

        if (mime) {
            Regex regex_config;
            regex_config.string = mime;
            compile_regex(&regex_config);
            if (!MATCH_REGEX_SIMPLE(regex_config, file_mime)) {
                continue;
            }
        }
        if (path) {
            Regex regex_config;
            regex_config.string = path;
            compile_regex(&regex_config);
            if (!MATCH_REGEX_SIMPLE(regex_config, filename)) {
                continue;
            }
        }

        found = true;
        parse_command_run(rules[i].command, argc - 2, &argv[2]);
    }

    if (!found) {
        printf("No previewer set for file:\n\n%s:\n    %s\n", basename(argv[1]),
               file_mime);
    } else {
        error("Every previewer failed.\n");
    }
    exit(EXIT_FAILURE);
}

void
parse_command_run(char *const *command, int32 argc, char **argv) {
    Array args = {0};
    args.arena_pos = args.arena;

    for (int32 i = 0; command[i]; i += 1) {
        char *argument = command[i];
        regmatch_t matches[MAX_EXTRAS + 1];

        if (MATCH_REGEX_SIMPLE(regex_filename, argument)) {
            array_push(&args, filename, 0);
            continue;
        }
        if (MATCH_SUBEXPRESSIONS(regex_extras, argument, matches)) {
            int32 extra_index = get_extra_number(argument, matches[1]);

            if (extra_index >= argc) {
                error("Extra argument %d not passed to piscou. Ignoring...\n",
                      extra_index);
                goto ignore;
            }
            array_push(&args, argv[extra_index], 0);
            continue;
        }
        if (MATCH_SUBEXPRESSIONS(regex_extras_more, argument, matches)) {
            char *pointer = args.arena_pos;
            int32 extra_length = 0;
            int32 final_length;
            strcpy(pointer, argument);
            do {
                char *argv_passed;
                int32 total_length;
                int32 start = matches[0].rm_so;
                int32 end = matches[0].rm_eo;
                int32 left = (int32)strlen(&pointer[end]) + 1;
                int32 extra_index = get_extra_number(pointer, matches[1]);

                if (extra_index >= argc) {
                    error("Extra argument %d not passed to piscou."
                          " Ignoring...\n",
                          extra_index);
                    goto ignore;
                }

                argv_passed = argv[extra_index];
                extra_length = (int32)strlen(argv_passed);
                total_length
                    = (int32)(pointer - args.arena_pos) + extra_length + left;
                if (total_length >= MAX_ARGUMENT_LENGTH) {
                    error("Too long argument. Max length is %d.\n",
                          MAX_ARGUMENT_LENGTH);
                    exit(EXIT_FAILURE);
                }

                memmove(&pointer[start + extra_length], &pointer[end],
                        (size_t)left);
                memcpy(&pointer[start], argv_passed, (size_t)extra_length);
                pointer += (extra_length + start);
            } while (MATCH_SUBEXPRESSIONS(regex_extras_more, pointer, matches));

            final_length = (int32)(pointer - args.arena_pos);
            array_push(&args, NULL, final_length);
            continue;
        }
        array_push(&args, argument, 0);
    ignore:
        continue;
    }
    if (PISCOU_BENCHMARK || PISCOU_DEBUG) {
        for (int32 i = 0; i < (args.len + 1); i += 1) {
            printf("args.array[%d] = %s\n", i, args.array[i]);
        }
    }
    if (args.array[0] == NULL) {
        error("Invalid command.\n");
        exit(EXIT_FAILURE);
    }
    execvp(args.array[0], args.array);
    {
        char full_command[MAX_ARGUMENT_LENGTH*MAX_ARGS];
        STRING_FROM_STRINGS(full_command, " ", args.array, args.len + 1);
        error("Error executing\n%s\n%s\n", full_command, strerror(errno));
    }
    return;
}

void
usage(FILE *stream) {
    fprintf(stream, "usage: piscou %%piscou-filename%%"
                    " [ %%piscou-extra0%% %%piscou-extra1%% ... ]\n");
    exit(stream != stdout);
}

int32
get_extra_number(char *string, regmatch_t pmatch) {
    char number_buffer[12] = {0};

    int32 start = pmatch.rm_so;
    int32 end = pmatch.rm_eo;
    int32 diff = end - start;
    int32 number;

    memcpy(number_buffer, string + start, (size_t)diff);
    number = atoi(number_buffer);
    return number;
}

void
array_push(Array *array, char *string, int32 length) {
    if (string) {
        array->array[array->len] = string;
    } else {
        array->array[array->len] = array->arena_pos;
        array->arena_pos += length;
    }
    array->len += 1;
    return;
}
