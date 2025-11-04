#if !defined(CONFIG_H)
#define CONFIG_H

#include <stdlib.h>

#define MAX_ARGS 16
#define MAX_EXTRAS 10
#define MAX_ARGUMENT_LENGTH 256
#define REGEX_FILENAME "#piscou-file#"
#define REGEX_EXTRAS "^#piscou-([0-9])#$"
#define REGEX_EXTRAS_MORE "#piscou-([0-9])#"

typedef struct Rule {
    char *match[2];
    char *command[MAX_ARGS];
} Rule;

static const Rule rules[] = {
/* mimetype            filename      command */
{{NULL,   "v[a-z0-9]{1,3}::." }, {"vfile.sh", "#piscou-file#"}},
{{NULL,   "g[a-z0-9]{1,3}::." }, {"gdir.sh", "#piscou-file#"}},
{{NULL,   ".+\\.fen$"         }, {"fen.sh", "#piscou-file#"}},
{{NULL,   ".+\\.mo$"          }, {"pygmentize", "#piscou-file#"}},
{{NULL,   ".+\\.blend$"       }, {"blend.sh", "#piscou-file#"}},
{{NULL,   ".+\\.csv"          }, {"csv.sh", "#piscou-file#"}},
{{NULL,   ".+\\.kicad_pro$"   }, {"kicad.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{"inode/directory",      NULL}, {"ls", "-1A", "--color", "#piscou-file#"}},
{{"inode/x-empty",        NULL}, {"/usr/bin/cat", "-A", "#piscou-file#"}},
{{"ms(word|-exce|-powe)", NULL}, {"printf", "💩💩💩💩💩💩💩💩💩\n%s\n💩💩💩💩💩💩💩💩💩", "#piscou-file#"}},
{{"opendoc.+spreadsheet", NULL}, {"ods.sh", "#piscou-file#"}},
{{"officed.+spreadsheet", NULL}, {"xlsx.sh", "#piscou-file#"}},
{{"office.+word",         NULL}, {"docx.sh", "#piscou-file#", "#piscou-0#"}},
{{"office.+pres",         NULL}, {"ppt.sh", "#piscou-file#", "#piscou-0#"}},
{{"opendocument",         NULL}, {"odt2txt", "#piscou-file#"}},
{{"appl.+/pdf",           NULL}, {"pdf.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{"appl.+/epub.+",        NULL}, {"epub.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{"appl.+/csv",           NULL}, {"csv.sh", "#piscou-file#"}},
{{"appl.+/json",          NULL}, {"head", "-n", "40", "#piscou-file#"}},
{{"appl.+/js",            NULL}, {"bat",  "#piscou-file#"}},
{{"appl.+/javascript",    NULL}, {"bat",  "#piscou-file#"}},
{{"appl.+/.*execu.+",     NULL}, {"objdump", "-T", "#piscou-file#", "#piscou-0#"}},
{{"appl.+/x-objec.+",     NULL}, {"objdump", "-t", "#piscou-file#", "#piscou-0#"}},
{{"appl.+/x-sharedlib",   NULL}, {"objdump", "-T", "#piscou-file#", "#piscou-0#"}},
{{"appl.+/zip",           NULL}, {"unzip", "-l", "#piscou-file#"}},
{{"appl.+/gzip",          NULL}, {"tar", "tf", "#piscou-file#"}},
{{"appl.+/x-7z-.+",       NULL}, {"7z", "l", "#piscou-file#"}},
{{"appl.+/x-subrip",      NULL}, {"/usr/bin/cat", "#piscou-file#"}},
{{"appl.+/mbox",          NULL}, {"bat", "-p", "--pager=never", "--color=always", "#piscou-file#"}},
{{NULL,             ".+\\.ff$"}, {"chafa", "#piscou-file#", "-s", "#piscou-0#x#piscou-1#"}},
{{NULL,            ".+\\.gif$"}, {"chafa", "#piscou-file#", "-s", "#piscou-0#x#piscou-1#"}},
{{NULL,           ".+\\.webp$"}, {"chafa", "#piscou-file#", "-s", "#piscou-0#x#piscou-1#"}},
{{NULL,              "\\.hdr$"}, {"hdr.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{NULL,          ".+\\.[1-9]$"}, {"man", "#piscou-file#"}},
{{NULL,           ".+\\.sent$"}, {"/usr/bin/cat", "#piscou-file#"}},
{{"image/.*dwg",          NULL}, {"stat", "#piscou-file#"}},
{{"image/.*xml",          NULL}, {"head", "-n", "40", "#piscou-file#"}},
{{"image/.*",             NULL}, {"stiv_draw", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{"image/.*",             NULL}, {"chafa", "#piscou-file#", "-s", "#piscou-0#x#piscou-1#"}},
{{"audio/.*",             NULL}, {"vid.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{"video/.*",             NULL}, {"vid.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{"appl[^/]+/x-matroska", NULL}, {"vid.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{"appl[^/]+/x-kicad-.*", NULL}, {"kicad.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{"appl[^/]+/octet-stream", NULL}, {"xxd", "#piscou-file#"}},
{{"text/.+",              NULL}, {"bat", "-p", "--pager=never", "--color=always", "#piscou-file#"}},
};

#endif
