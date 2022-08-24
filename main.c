#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <regex.h>
#include <magic.h>

#include "util.h"

char *filename = NULL;
char *extras[9] = {NULL};
char config[256];
FILE *conf;

void open_config(void) {
    printf("open_config()\n");
   char *cache = NULL;
   char *piscou = "piscou/piscou.conf";

   if (!(cache = getenv("XDG_CONFIG_HOME"))) {
       fprintf(stderr, "XDG_CONFIG_HOME needs to be set\n");
       exit(1);
   }
    printf("cache = %s\n", cache);

   snprintf(config, sizeof(config), "%s/%s", cache, piscou);
   config[255] = '\0';

    printf("config = %s\n", config);

   if (!(conf = fopen(config, "r"))) {
       fprintf(stderr, "%s\n", strerror(errno));
       exit(1);
   }
}

void preview(void) {
   open_config();
   char buf[256];
   char *pbuf;
   char *mime_conf = NULL;
   char *mime_file = NULL;
   char *comm = NULL;

   char *cargs[100] = {NULL};
   size_t i = 0;

   magic_t m;
   m = magic_open(MAGIC_MIME_TYPE);
   magic_load(m, NULL);
   mime_file = magic_file(m, filename);
   regex_t r;
   int v;
   bool match = false;

   while (fgets(buf, sizeof(buf), conf)) {
       pbuf = buf;

       while ((*pbuf == ' ') || (*pbuf == '\t'))
           pbuf++;
       if ((*pbuf == '#') || (*pbuf == '\n'))
           continue;

       mime_conf = strtok(pbuf, " ");
       if (!strncmp(mime_conf, "fpath", 5)) {
           printf("fpath!\n");
           mime_conf = strtok(NULL, " \t\n");
       }
       v = regcomp(&r, mime_conf, REG_EXTENDED);
       if (v != 0) {
           fprintf(stderr, "Error creating regex for mime_conf %s\n", mime_conf);
           continue;
       }

        if(!strncmp(mime_file, mime_conf, 10)) {
            printf("MAGIC: %s!\n", mime_file);
        } else {
            printf("NO magic: %s != %s\n", mime_file, mime_conf);
        }

       match = regexec(&r, filename, 0, NULL, 0) == 0;
       if (match) {
           i = 0;
           while ((comm = strtok(NULL, " \t\n"))) {
               printf("comm: %s\n", comm);
               cargs[i] = comm;
               i += 1;
           }
           for (i = 0; i < sizeof(cargs); i++) {
               if (cargs[i] == NULL)
                   break;
               printf("cargs[%ld]=%s\n", i, cargs[i]);
               if (!strncmp(cargs[i], "%piscou-filename%", 100)) {
                    cargs[i] = filename;
               }
           }
           execvp(cargs[0], cargs);
           break;
       }
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
