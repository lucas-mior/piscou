// SPDX-License-Identifier: AGPL
// Copyright (c) 2026 Lucas Mior

#include <magic.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <libgen.h>
#include <errno.h>

#define CBASE_IMPLEMENT
#include "cbase.h"

#if !defined(DEBUGGING)
  #define DEBUGGING 0
#endif

#include "meta_regex/meta_regex.h"
#include "meta_regex/meta_match.c"
#include "gen/config2.h"

#define PISCOU_META_MATCHERS \
    (enum Matcher) \
    (MATCHER_BTNFA | MATCHER_TDFA | MATCHER_LAZY_DFA \
     | MATCHER_STATIC_DFA)

#define MATCH_SUBEXPRESSIONS(R, S, PMATCHES) \
    (meta_regex_match((R), (uint8 *)(S), strlen32(S), (PMATCHES), \
                      LENGTH(PMATCHES), PISCOU_META_MATCHERS) == 0)

#define MATCH_REGEX_SIMPLE(R, S) \
    (meta_regex_match((R), (uint8 *)(S), strlen32(S), NULL, 0, \
                      PISCOU_META_MATCHERS) == 0)

typedef struct Array {
    char arena[MAX_EXTRAS*MAX_ARGUMENT_LENGTH];
    char *array[MAX_ARGS];
    char *arena_pos;
    int32 len;
    int32 unused;
} Array;

static inline int32 get_extra_number(char *, regmatch_t);
static inline void array_push(Array *, char *, int64);
static inline void parse_command_run(char *const *, int64, char **);
static void usage(FILE *) __attribute__((noreturn));

static char *filename;

static char *regex_filename_str = "#piscou-file#";

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

    for (int64 i = 0; i < LENGTH(rules); i += 1) {
        MetaRegex *mime_pat = rules[i].match[0];
        MetaRegex *path_pat = rules[i].match[1];

        if (mime_pat == NULL && path_pat == NULL) {
            continue;
        }

        if (mime_pat) {
            if (!MATCH_REGEX_SIMPLE(mime_pat, (char *)file_mime)) {
                continue;
            }
        }

        if (path_pat) {
            if (!MATCH_REGEX_SIMPLE(path_pat, filename)) {
                continue;
            }
        }

        found = true;
        parse_command_run(rules[i].command, argc - 2, &argv[2]);
    }

    if (!found) {
        printf("No previewer set for file:\n\n%s:\n    %s\n",
               basename(argv[1]), file_mime);
    } else {
        error("Every previewer failed.\n");
    }
    exit(EXIT_FAILURE);
}

void
parse_command_run(char *const *command, int64 argc, char **argv) {
    Array args = {0};
    args.arena_pos = args.arena;

    for (int64 i = 0; command[i]; i += 1) {
        char *argument = command[i];
        regmatch_t matches[MAX_EXTRAS + 1];

        if (!strcmp(regex_filename_str, argument)) {
            array_push(&args, filename, 0);
            continue;
        }
        if (MATCH_SUBEXPRESSIONS(piscou_regex_extras, argument, matches)) {
            int32 extra_index = get_extra_number(argument, matches[1]);
            if (extra_index >= argc) {
                error("Extra argument %d not passed to piscou. Ignoring...\n",
                      extra_index);
                goto ignore;
            }
            array_push(&args, argv[extra_index], 0);
            continue;
        }
        if (MATCH_SUBEXPRESSIONS(piscou_regex_extras_more, argument, matches)) {
            char *pointer = args.arena_pos;
            int64 extra_length = 0;
            int64 final_length;
            strcpy(pointer, argument);
            do {
                char *argv_passed;
                int64 total_length;
                int64 start = matches[0].rm_so;
                int64 end = matches[0].rm_eo;
                int64 left = strlen32(&pointer[end]) + 1;
                int32 extra_index = get_extra_number(pointer, matches[1]);

                if (extra_index >= argc) {
                    error("Extra argument %d not passed to piscou. Ignoring...\n",
                          extra_index);
                    goto ignore;
                }

                argv_passed = argv[extra_index];
                extra_length = strlen32(argv_passed);
                total_length = (int64)(pointer - args.arena_pos)
                                       + extra_length + left;
                if (total_length >= MAX_ARGUMENT_LENGTH) {
                    error("Too long argument. Max length is %d.\n",
                          MAX_ARGUMENT_LENGTH);
                    exit(EXIT_FAILURE);
                }

                memmove64(&pointer[start + extra_length], &pointer[end], left);
                memcpy64(&pointer[start], argv_passed, extra_length);
                pointer += (extra_length + start);
            } while (MATCH_SUBEXPRESSIONS(piscou_regex_extras_more,
                                          pointer, matches));

            final_length = (int64)(pointer - args.arena_pos);
            array_push(&args, NULL, final_length);
            continue;
        }
        array_push(&args, argument, 0);
    ignore:
        continue;
    }
    
    if (DEBUGGING) {
        for (int32 i = 0; i < args.len; i += 1) {
            printf("args.array[%d] = %s\n", i, args.array[i]);
        }
    }
    
    if (args.array[0] == NULL) {
        error("Invalid command.\n");
        exit(EXIT_FAILURE);
    }
    
    if (!DEBUGGING) {
        char full_command[MAX_ARGUMENT_LENGTH*MAX_ARGS];

        execvp(args.array[0], args.array);

        STRING_FROM_ARRAY(full_command, " ", args.array, args.len);
        error("Error executing\n%s\n%s\n", full_command, strerror(errno));
    }
    return;
}

void
usage(FILE *stream) {
    fprintf(stream,
            "usage: piscou #piscou-file# [ #piscou-0# #piscou-1# ... ]\n");
    exit(stream != stdout);
}

int32
get_extra_number(char *string, regmatch_t pmatch) {
    char number_buffer[12] = {0};
    int64 start = pmatch.rm_so;
    int64 end = pmatch.rm_eo;
    int64 diff = end - start;
    memcpy64(number_buffer, string + start, diff);
    return (int32)atoi(number_buffer);
}

void
array_push(Array *array, char *string, int64 length) {
    if (string) {
        array->array[array->len] = string;
    } else {
        array->array[array->len] = array->arena_pos;
        array->arena_pos += length;
    }
    array->len += 1;
    array->array[array->len] = NULL;
    return;
}
