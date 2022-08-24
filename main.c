#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <regex.h>

#include "util.h"

char *filename = NULL;
char* extras[9] = {NULL};
char *config = "/home/lucas/.config/piscou/piscou.conf";
FILE *conf;

void preview(void) {
   char buf[256];
   char *mime = buf;
   char *comm = buf;

   if (!(conf = fopen(config, "r"))) {
       fprintf(stderr, "%s\n", strerror(errno));
       exit(1);
   }
   printf("\n\nopened config file!\n");

   /* regex_t r; */
   /* int value; */
   while (fgets(buf, sizeof(buf), conf)) {
       comm = mime = buf;
       comm = strstr(buf, " ");
       mime[comm - mime - 1] = '\0';
       printf("comm: %s\n", comm);
       printf("mime: %s\n", mime);
   }
   fclose(conf);
}

int main(int argc, char *argv[]) {
    int option;
    size_t i = 0;
    while((option = getopt(argc, argv, "hx:")) != -1){ //get option from the getopt() method
      switch(option){
         case 'h':
            usage(stdout);
            break;
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
       usage(stderr);

   fprintf(stdout, "filename: %s\n", filename);
   exit(0);

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

   preview();
   return 0;
}
