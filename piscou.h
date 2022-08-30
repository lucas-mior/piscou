#ifndef piscou_h
#define piscou_h

typedef struct Rule {
    char *mime;
    char *args[10];
} Rule;

Rule rules[] = {
{"fpath v\\S{1,3}::", {"vfile.sh", "%piscou-filename%"}},
{"fpath g\\S{1,3}::", {"gdir.sh", "%piscou-filename%"}},
{"text/.*", {"bat", "-p", "--pager=never", "--color=always", "%piscou-filename%"}},
{"inode/directory", {"ls", "-1A", "--color", "%piscou-filename%"}},
{"ms(word|-excel|-power)", {"printf", "💩💩💩💩💩💩💩💩💩\n%s\n💩💩💩💩💩💩💩💩💩", "%piscou-filename%"}},
{"opendoc.+spreadsheet", {"ods.sh", "%piscou-filename%"}},
{"officed.+spreadsheet", {"xlsx.sh", "%piscou-filename%"}},
{"office.+word",         {"docx.sh", "%piscou-filename%", "%piscou-extra0%"}},
{"office.+pres", {"ppt.sh", "%piscou-filename%", "%piscou-extra0%"}},
{"opendocument", {"odt2txt", "%piscou-filename%"}},
{"application/pdf", {"pdf.sh", "%piscou-filename%", "%piscou-extra0%", "%piscou-extra1%", "%piscou-extra2%", "%piscou-extra3%"}},
{"application/csv", {"column", "-t", "-s", ",", "%piscou-filename%"}},
{"application/json", {"head -n 40", "%piscou-filename%"}},
{"application/.*execu.+",  {"execu.sh", "%piscou-filename%", "%piscou-extra0%"}},
{"fpath .+\\.ff$", {"stiv", "%piscou-filename%", "%piscou-extra0%", "%piscou-extra1%", "%piscou-extra2%", "%piscou-extra3%"}},
{"fpath .+\\.[1-9]$", {"man", "%piscou-filename%"}},
{"image/.*dwg", {"stat", "%piscou-filename%"}},
{"image/.*xml", {"head -n 40", "%piscou-filename%"}},
{"image/.*", {"stiv", "%piscou-filename%", "%piscou-extra0%", "%piscou-extra1%", "%piscou-extra2%", "%piscou-extra3%"}},
{"audio/.*", {"vid.sh", "%piscou-filename%", "%piscou-extra0%", "%piscou-extra1%", "%piscou-extra2%", "%piscou-extra3%"}},
{"video/.*", {"vid.sh", "%piscou-filename%", "%piscou-extra0%", "%piscou-extra1%", "%piscou-extra2%", "%piscou-extra3%"}},
{".+", {"file", "--mime-type", "%piscou-filename%"}}
};

#endif /* piscou_h */
