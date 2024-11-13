#ifndef CONFIG_H
#define CONFIG_H

#include <stdlib.h>

#define MAX_ARGS 16
#define MAX_EXTRAS 10
#define MAX_ARGUMENT_LENGTH 256
#define REGEX_FILENAME "%piscou-filename%"
#define REGEX_EXTRAS "^%piscou-extra([0-9])%$"
#define REGEX_EXTRAS_MORE "%piscou-extra([0-9])%"

typedef struct Rule {
    char *match[2];
    char *command[MAX_ARGS];
} Rule;

static const Rule rules[] = {
/* mimetype            filename      command */
{{NULL,   "v[a-z0-9]{1,3}::." }, {"vfile.sh", "%piscou-filename%"}},
{{NULL,   "g[a-z0-9]{1,3}::." }, {"gdir.sh", "%piscou-filename%"}},
{{NULL,   ".+\\.fen$"         }, {"fen.sh", "%piscou-filename%"}},
{{NULL,   ".+\\.mo$"          }, {"pygmentize", "%piscou-filename%"}},
{{NULL,   ".+\\.blend$"       }, {"blend.sh", "%piscou-filename%"}},
{{NULL,   ".+\\.csv"          }, {"csv.sh", "%piscou-filename%"}},
{{NULL,   ".+\\.kicad_pro$"   }, {"kicad.sh", "%piscou-filename%", "%piscou-extra0%", "%piscou-extra1%", "%piscou-extra2%", "%piscou-extra3%"}},
{{"inode/directory",      NULL}, {"ls", "-1A", "--color", "%piscou-filename%"}},
{{"inode/x-empty",        NULL}, {"/usr/bin/cat", "-A", "%piscou-filename%"}},
{{"ms(word|-exce|-powe)", NULL}, {"printf", "💩💩💩💩💩💩💩💩💩\n%s\n💩💩💩💩💩💩💩💩💩", "%piscou-filename%"}},
{{"opendoc.+spreadsheet", NULL}, {"ods.sh", "%piscou-filename%"}},
{{"officed.+spreadsheet", NULL}, {"xlsx.sh", "%piscou-filename%"}},
{{"office.+word",         NULL}, {"docx.sh", "%piscou-filename%", "%piscou-extra0%"}},
{{"office.+pres",         NULL}, {"ppt.sh", "%piscou-filename%", "%piscou-extra0%"}},
{{"opendocument",         NULL}, {"odt2txt", "%piscou-filename%"}},
{{"appl.+/pdf",           NULL}, {"pdf.sh", "%piscou-filename%", "%piscou-extra0%", "%piscou-extra1%", "%piscou-extra2%", "%piscou-extra3%"}},
{{"appl.+/csv",           NULL}, {"csv.sh", "%piscou-filename%"}},
{{"appl.+/json",          NULL}, {"head", "-n", "40", "%piscou-filename%"}},
{{"appl.+/js",            NULL}, {"bat",  "%piscou-filename%"}},
{{"appl.+/.*execu.+",     NULL}, {"objdump", "-T", "%piscou-filename%", "%piscou-extra0%"}},
{{"appl.+/x-objec.+",     NULL}, {"objdump", "-t", "%piscou-filename%", "%piscou-extra0%"}},
{{"appl.+/x-sharedlib",   NULL}, {"objdump", "-T", "%piscou-filename%", "%piscou-extra0%"}},
{{"appl.+/zip",           NULL}, {"unzip", "-l", "%piscou-filename%"}},
{{"appl.+/gzip",          NULL}, {"tar", "tf", "%piscou-filename%"}},
{{"appl.+/x-7z-.+",       NULL}, {"7z", "l", "%piscou-filename%"}},
{{"appl.+/x-subrip",      NULL}, {"/usr/bin/cat", "%piscou-filename%"}},
{{NULL,             ".+\\.ff$"}, {"chafa", "%piscou-filename%", "-s", "%piscou-extra0%x%piscou-extra1%"}},
{{NULL,            ".+\\.gif$"}, {"chafa", "%piscou-filename%", "-s", "%piscou-extra0%x%piscou-extra1%"}},
{{NULL,           ".+\\.webp$"}, {"chafa", "%piscou-filename%", "-s", "%piscou-extra0%x%piscou-extra1%"}},
{{NULL,              "\\.hdr$"}, {"hdr.sh", "%piscou-filename%", "%piscou-extra0%", "%piscou-extra1%", "%piscou-extra2%", "%piscou-extra3%"}},
{{NULL,          ".+\\.[1-9]$"}, {"man", "%piscou-filename%"}},
{{NULL,           ".+\\.sent$"}, {"/usr/bin/cat", "%piscou-filename%"}},
{{"image/.*dwg",          NULL}, {"stat", "%piscou-filename%"}},
{{"image/.*xml",          NULL}, {"head", "-n", "40", "%piscou-filename%"}},
{{"image/.*",             NULL}, {"stiv_draw", "%piscou-filename%", "%piscou-extra0%", "%piscou-extra1%", "%piscou-extra2%", "%piscou-extra3%"}},
{{"image/.*",             NULL}, {"chafa", "%piscou-filename%", "-s", "%piscou-extra0%__%piscou-extra1%__%piscou-extra2%__%piscou-extra3%"}},
{{"audio/.*",             NULL}, {"vid.sh", "%piscou-filename%", "%piscou-extra0%", "%piscou-extra1%", "%piscou-extra2%", "%piscou-extra3%"}},
{{"video/.*",             NULL}, {"vid.sh", "%piscou-filename%", "%piscou-extra0%", "%piscou-extra1%", "%piscou-extra2%", "%piscou-extra3%"}},
{{"appl[^/]+/x-matroska", NULL}, {"vid.sh", "%piscou-filename%", "%piscou-extra0%", "%piscou-extra1%", "%piscou-extra2%", "%piscou-extra3%"}},
{{"appl[^/]+/x-kicad-.*", NULL}, {"kicad.sh", "%piscou-filename%", "%piscou-extra0%", "%piscou-extra1%", "%piscou-extra2%", "%piscou-extra3%"}},
{{"text/.+",              NULL}, {"bat", "-p", "--pager=never", "--color=always", "%piscou-filename%"}},
};

#endif
