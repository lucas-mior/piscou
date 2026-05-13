#if !defined(CONFIG_H)
#define CONFIG_H

#include <stdlib.h>
#include "meta_regex/meta_regex.h"

#define MAX_ARGS 16
#define MAX_EXTRAS 10
#define MAX_ARGUMENT_LENGTH 256

typedef struct Rule {
    MetaRegex match[2];
    char *command[MAX_ARGS];
} Rule;

static Rule rules[] = {
/* mimetype                    filename          command */
{{R(NULL),   R("v[a-z0-9]{1,3}::.") }, {"vfile.sh", "#piscou-file#"}},
{{R(NULL),   R("g[a-z0-9]{1,3}::.") }, {"gdir.sh", "#piscou-file#"}},
{{R(NULL),   R(".+\\.fen$")         }, {"fen.sh", "#piscou-file#"}},
{{R(NULL),   R(".+\\.mo$")          }, {"pygmentize", "#piscou-file#"}},
{{R(NULL),   R(".+\\.blend$")       }, {"blend.sh", "#piscou-file#"}},
{{R(NULL),   R(".+\\.csv")          }, {"csv.sh", "#piscou-file#"}},
{{R(NULL),   R(".+\\.kicad_pro$")   }, {"kicad.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{R("inode/directory"),      R(NULL)}, {"ls", "-1A", "--color", "#piscou-file#"}},
{{R("inode/x-empty"),        R(NULL)}, {"/usr/bin/cat", "-A", "#piscou-file#"}},
{{R("ms(word|-exce|-powe)"), R(NULL)}, {"printf", "\n%s\n", "#piscou-file#"}},
{{R("opendoc.+spreadsheet"), R(NULL)}, {"ods.sh", "#piscou-file#"}},
{{R("officed.+spreadsheet"), R(NULL)}, {"xlsx.sh", "#piscou-file#"}},
{{R("office.+word"),         R(NULL)}, {"docx.sh", "#piscou-file#", "#piscou-0#"}},
{{R("office.+pres"),         R(NULL)}, {"ppt.sh", "#piscou-file#", "#piscou-0#"}},
{{R("opendocument"),         R(NULL)}, {"odt2txt", "#piscou-file#"}},
{{R("appl.+/pdf"),           R(NULL)}, {"pdf.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{R("appl.+/epub.+"),        R(NULL)}, {"epub.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{R("appl.+/csv"),           R(NULL)}, {"csv.sh", "#piscou-file#"}},
{{R("appl.+/json"),          R(NULL)}, {"head", "-n", "40", "#piscou-file#"}},
{{R("appl.+/js"),            R(NULL)}, {"bat",  "#piscou-file#"}},
{{R("appl.+/javascript"),    R(NULL)}, {"bat",  "#piscou-file#"}},
{{R("appl.+/.*execu.+"),     R(NULL)}, {"objdump", "-T", "#piscou-file#", "#piscou-0#"}},
{{R("appl.+/x-objec.+"),     R(NULL)}, {"objdump", "-t", "#piscou-file#", "#piscou-0#"}},
{{R("appl.+/x-sharedlib"),   R(NULL)}, {"objdump", "-T", "#piscou-file#", "#piscou-0#"}},
{{R("appl.+/zip"),           R(NULL)}, {"unzip", "-l", "#piscou-file#"}},
{{R("appl.+/gzip"),          R(NULL)}, {"tar", "tf", "#piscou-file#"}},
{{R("appl.+/x-7z-.+"),       R(NULL)}, {"7z", "l", "#piscou-file#"}},
{{R("appl.+/x-subrip"),      R(NULL)}, {"/usr/bin/cat", "#piscou-file#"}},
{{R("appl.+/mbox"),          R(NULL)}, {"bat", "-p", "--pager=never", "--color=always", "#piscou-file#"}},
{{R(NULL),               R("\\.hdr$")}, {"hdr.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{R(NULL),           R(".+\\.[1-9]$")}, {"man", "#piscou-file#"}},
{{R(NULL),           R(".+\\.sent$")}, {"/usr/bin/cat", "#piscou-file#"}},
{{R("image/.*dwg"),          R(NULL)}, {"stat", "#piscou-file#"}},
{{R("image/.*xml"),          R(NULL)}, {"head", "-n", "40", "#piscou-file#"}},
{{R(NULL),            R(".+\\.gif$")}, {"stiv_draw", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{R(NULL),           R(".+\\.webp$")}, {"stiv_draw", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{R("image/.*"),             R(NULL)}, {"stiv_draw", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{R("image/.*"),             R(NULL)}, {"chafa", "#piscou-file#", "--size=#piscou-0#x#piscou-1#"}},
{{R("audio/.*"),             R(NULL)}, {"vid.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{R("video/.*"),             R(NULL)}, {"vid.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{R("appl[^/]+/x-matroska"), R(NULL)}, {"vid.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{R("appl[^/]+/x-kicad-.*"), R(NULL)}, {"kicad.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{R("appl[^/]+/octet-stream"), R(NULL)}, {"xxd", "#piscou-file#"}},
{{R("text/x-tex"),        R(".+\\.pgf$")}, {"pgf.sh", "#piscou-file#", "#piscou-0#", "#piscou-1#", "#piscou-2#", "#piscou-3#"}},
{{R("text/.+"),              R(NULL)}, {"bat", "-p", "--pager=never", "--color=always", "#piscou-file#"}},
};

#endif
