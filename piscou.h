#ifndef piscou_h
#define piscou_h

typedef struct Command {
    char *mime;
    char *args[10];
} Command;

Command commands[] = {
    {.mime = "text/*",  .args = {"bat", "-p", "main.c"}},
    {.mime = "image/*", .args = {"echo", "image", "%piscou-filename%"}},
    {.mime = "video/*", .args = {"vid.sh", "%piscou-filename"}},
};

#endif /* piscou_h */
