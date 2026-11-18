#define CBASE_IMPLEMENT
#include "cbase.h"

#define PISCOU_REGEX_IMPLEMENT
#include "piscou_regex.h"

#if PISCOU_REGEX_BACKEND_LIBC
#include "config.h"
#else
#include "gen/config2.h"
#endif

typedef struct RegexFixture {
    char *mime;
    char *path;
} RegexFixture;

static RegexFixture fixtures[] = {
    {"text/plain", "sample.csv"},
    {"text/plain", "sample.fen"},
    {"text/plain", "sample.unknown"},
    {"image/png", "picture.png"},
    {"image/gif", "picture.gif"},
    {"inode/directory", "folder"},
    {"inode/x-empty", "empty"},
    {"application/pdf", "document.pdf"},
    {"application/javascript", "script.js"},
    {"application/json", "file.json"},
    {"application/zip", "archive.zip"},
    {"application/x-7z-compressed", "archive.7z"},
    {"application/vnd.ms-excel", "table.xls"},
    {"application/vnd.oasis.opendocument.spreadsheet", "table.ods"},
    {"audio/mpeg", "music.mp3"},
    {"video/mp4", "movie.mp4"},
    {"text/x-tex", "drawing.pgf"},
    {"text/plain", "drawing.pgf"},
    {"text/x-tex", "drawing.txt"},
    {"text/plain", "v23::."},
    {"text/plain", "gdir::."},
    {"text/plain", "manual.8"},
    {"application/x-hdf5", "dataset.h5"},
    {"", ""},
};

static char *extra_samples[] = {
    "#piscou-0#",
    "#piscou-9#",
    "before#piscou-2#after",
    "#piscou-0#x#piscou-1#",
    "no placeholder",
    "#piscou-12#",
};

int
main(void) {
    PiscouRegexMatch matches[3];

    for (int64 i = 0; i < LENGTH(rules); i += 1) {
        PiscouRegex *mime = rules[i].match[0];
        PiscouRegex *path = rules[i].match[1];

        for (int64 j = 0; j < LENGTH(fixtures); j += 1) {
            char *mime_string = fixtures[j].mime;
            char *path_string = fixtures[j].path;
            bool mime_match = mime == NULL ||
                piscou_regex_match(mime, mime_string, NULL, 0);
            bool path_match = path == NULL ||
                piscou_regex_match(path, path_string, NULL, 0);

            fmt_printf("%d %d %d %d\n",
                       (int)i, (int)j, (int)mime_match, (int)path_match);
        }
    }

    for (int64 i = 0; i < LENGTH(extra_samples); i += 1) {
        bool direct = piscou_regex_match(piscou_regex_extras,
                                         extra_samples[i],
                                         matches, LENGTH(matches));
        fmt_printf("direct %d %d", (int)i, (int)direct);
        for (int64 j = 0; j < LENGTH(matches); j += 1) {
            fmt_printf(" %d %d", (int)matches[j].rm_so, (int)matches[j].rm_eo);
        }
        fmt_printf("\n");

        bool embedded = piscou_regex_match(piscou_regex_extras_more,
                                           extra_samples[i],
                                           matches, LENGTH(matches));
        fmt_printf("embedded %d %d", (int)i, (int)embedded);
        for (int64 j = 0; j < LENGTH(matches); j += 1) {
            fmt_printf(" %d %d", (int)matches[j].rm_so, (int)matches[j].rm_eo);
        }
        fmt_printf("\n");
    }

#if PISCOU_REGEX_BACKEND_LIBC
    piscou_regex_cleanup();
#endif
    return 0;
}
