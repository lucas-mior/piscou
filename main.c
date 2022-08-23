#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void usage(void) {
    printf("usage: piscou <filename> [extras...]\n"
           "ARGUMENTS:\n"
           "filename: file to preview\n"
           "extras: extra arguments passed to command\n");
    exit(0);
}

char *filename = NULL;
char* extras[9] = {NULL};

int main(int argc, char *argv[]) {
    int option;
    size_t i = 0;
    while((option = getopt(argc, argv, "hx:")) != -1){ //get option from the getopt() method
      switch(option){
         case 'h':
            usage();
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
   if (argv[optind] != NULL)
       filename = argv[optind];
   else
       usage();

   printf("filename: %s\n", filename);

   optind += 1;
   while (optind < argc) {
       extras[i] = argv[optind];
       optind += 1;
       i += 1;
   }
   i = 0;

   printf("extras: ");
   while (extras[i]) {
       printf("%s ", extras[i]);
       i += 1;
   }
}
