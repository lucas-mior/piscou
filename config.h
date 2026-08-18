#if !defined(CONFIG_H)
#define CONFIG_H

#include "cbase.h"
#include "meta_regex/src/meta_regex.h"

#define MAX_ARGS 16
#define MAX_EXTRAS 10
#define MAX_ARGUMENT_LENGTH 256

typedef struct Rule {
    MetaRegex *match[2];
    char *command[MAX_ARGS];
} Rule;

static MetaRegex *piscou_regex_extras = R("^#piscou-([0-9])#$");
static MetaRegex *piscou_regex_extras_more = R("#piscou-([0-9])#");

static Rule rules[] = {
/* mimetype           filename    command */
{{NULL,    R("v[a-z0-9]{1,3}::.")}, {"vfile.sh", "#piscou-file#"}},
{{NULL,    R("g[a-z0-9]{1,3}::.")}, {"gdir.sh", "#piscou-file#"}},
{{NULL,    R(".+\\.fen$")        }, {"fen.sh", "#piscou-file#"}},
{{NULL,    R(".+\\.mo$")         }, {"pygmentize", "#piscou-file#"}},
{{NULL,    R(".+\\.blend$")      }, {"blend.sh", "#piscou-file#"}},
{{NULL,    R(".+\\.csv")         }, {"csv.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{NULL,    R(".+\\.dot")         }, {"dot.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{NULL,    R(".+\\.kicad_pro$")  }, {"kicad.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{NULL,    R(".+\\.sed$")        }, {"pygmentize", "#piscou-file#"}},
{{R("inode/directory"),      NULL}, {"ls", "-1A", "--color", "#piscou-file#"}},
{{R("inode/x-empty"),        NULL}, {"/usr/bin/cat", "-A", "#piscou-file#"}},
{{R("ms(word|-exce|-powe)"), NULL}, {"printf", "\n%s\n", "#piscou-file#"}},
{{R("opendoc.+spreadsheet"), NULL}, {"ods.sh", "#piscou-file#"}},
{{R("officed.+spreadsheet"), NULL}, {"xlsx.sh", "#piscou-file#"}},
{{R("office.+word"),         NULL}, {"docx.sh", "#piscou-file#", "#piscou-0#"}},
{{R("office.+pres"),         NULL}, {"ppt.sh", "#piscou-file#", "#piscou-0#"}},
{{R("opendocument"),         NULL}, {"odt2txt", "#piscou-file#"}},
{{R("appl.+/pdf"),           NULL}, {"pdf.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{R("appl.+/epub.+"),        NULL}, {"epub.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{R("appl.+/csv"),           NULL}, {"csv.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{R("appl.+/js"),            NULL}, {"bat",  "#piscou-file#"}},
{{R("appl.+/javascript"),    NULL}, {"bat",  "#piscou-file#"}},
{{R("appl.+/.*execu.+"),     NULL}, {"objdump", "-T", "#piscou-file#", "#piscou-0#"}},
{{R("appl.+/x-objec.+"),     NULL}, {"objdump", "-t", "#piscou-file#", "#piscou-0#"}},
{{R("appl.+/x-sharedlib"),   NULL}, {"objdump", "-T", "#piscou-file#", "#piscou-0#"}},
{{R("appl.+/zip"),           NULL}, {"unzip", "-l", "#piscou-file#"}},
{{R("appl.+/gzip"),          NULL}, {"tar", "tf", "#piscou-file#"}},
{{R("appl.+/x-7z-.+"),       NULL}, {"7z", "l", "#piscou-file#"}},
{{R("appl.+/x-subrip"),      NULL}, {"/usr/bin/cat", "#piscou-file#"}},
{{R("appl.+/mbox"),          NULL}, {"bat", "-p", "--pager=never", "--color=always", "#piscou-file#"}},
{{NULL,               R("\\.hdr$")}, {"hdr.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{NULL,           R(".+\\.[1-9]$")}, {"man", "#piscou-file#"}},
{{NULL,           R(".+\\.sent$")}, {"/usr/bin/cat", "#piscou-file#"}},
{{R("image/.*dwg"),          NULL}, {"stat", "#piscou-file#"}},
{{R("image/.*xml"),          NULL}, {"head", "-n", "40", "#piscou-file#"}},
{{NULL,            R(".+\\.gif$")}, {"stiv_draw", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{NULL,           R(".+\\.webp$")}, {"stiv_draw", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{R("image/.*"),             NULL}, {"stiv_draw", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{R("image/.*"),             NULL}, {"chafa", "#piscou-file#", "--size=#piscou-0#x#piscou-1#"}},
{{R("audio/.*"),             NULL}, {"vid.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{R("video/.*"),             NULL}, {"vid.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{R("appl[^/]+/x-matroska"), NULL}, {"vid.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{R("appl[^/]+/x-kicad-.*"), NULL}, {"kicad.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{R("appl[^/]+/octet-stream"), NULL}, {"xxd", "#piscou-file#"}},
{{R("text/x-tex"),    R(".+\\.pgf$")}, {"pgf.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{R("text/.+"),              NULL}, {"bat", "-p", "--pager=never", "--color=always", "#piscou-file#"}},
{{R("appl.+/json"),          NULL}, {"bat", "-p", "--pager=never", "--color=always", "#piscou-file#"}},
{{R("appl.+/x-hdf5"),          NULL}, {"hdf5.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
};

#endif
