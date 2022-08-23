#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void usage(void) {
    printf("usage: piscou <filename> [extras...]\n"
           "ARGUMENTS:\n"
           "filename: file to preview\n"
           "extras: extra arguments passed to command\n");
}

int main(int argc, char *argv[]) {
    printf("Hello from %s!\n", argv[0]);
    int option;
    while((option = getopt(argc, argv, "hx:")) != -1){ //get option from the getopt() method
          switch(option){
             case 'h':
                usage();
                exit(0);
             case 'f': //here f is used for some file name
                printf("Given File: %s\n", optarg);
                break;
             case 'x': //here f is used for some file name
                printf("Given File: %s\n", optarg);
                break;
             case ':':
                printf("option needs a value\n");
                break;
             case '?': //used for some unknown options
                printf("unknown option: %c\n", optopt);
                break;
          }
       }
}
