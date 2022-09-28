#ifndef piscou_h
#define piscou_h

typedef struct Rule {
    char *mime;
    char *args[10];
} Rule;

static Rule rules[] = {
{".+", {"file", "--mime-type", "%piscou-filename%"}}
};

#endif /* piscou_h */
