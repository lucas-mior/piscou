#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <regex.h>

#include "util.h"

char *filename = NULL;
char *extras[9] = {NULL};
char *config = "/home/lucas/.config/piscou/piscou.conf";
FILE *conf;

void preview(void) {
   char *cache = NULL;
   char *piscou = "piscou/piscou.conf";
   char config[256];

   if (!(cache = getenv("XDG_CONFIG_HOME"))) {
       fprintf(stderr, "XDG_CONFIG_HOME needs to be set\n");
       exit(1);
   }

   snprintf(config, sizeof(config), "%s/%s", cache, piscou);
   config[255] = '\0';

   if (!(conf = fopen(config, "r"))) {
       fprintf(stderr, "%s\n", strerror(errno));
       exit(1);
   }
   printf("\n\nopened config file!\n");
   char buf[256];
   char *mime = buf;
   char *comm = buf;

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
    while((option = getopt(argc, argv, "h")) != -1){ //get option from the getopt() method
      switch(option){
         case 'h':
            usage(stdout);
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

   optind += 1;
   while (optind < argc) {
       extras[i] = argv[optind];
       optind += 1;
       i += 1;
   }

   fprintf(stdout, "extras: %s\n", extras[0]);

   preview();
   return 0;
}
