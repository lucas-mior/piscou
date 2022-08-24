#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void usage(FILE *stream) {
    fprintf(stream,
            "usage: piscou <filename> [extras...]\n"
            "ARGUMENTS:\n"
            "filename: file to preview\n"
            "extras: extra arguments passed to command\n");
    exit(1);
}
