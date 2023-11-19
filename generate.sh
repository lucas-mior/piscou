#!/bin/sh

index_mime="index_mime.c"
index_path="index_path.c"

echo "char *commands[][16] = {" > commands.h

echo "
static int index_mime(const char *YYCURSOR)
{
    const char *YYMARKER;
    /*!re2c
       re2c:define:YYCTYPE = char;
       re2c:yyfill:enable = 0;
" > "$index_mime"

echo "
static int index_path(const char *YYCURSOR)
{
    const char *YYMARKER;
    /*!re2c
       re2c:define:YYCTYPE = char;
       re2c:yyfill:enable = 0;
" > "$index_path"

i=0
while read line; do
    regex_mime="$(echo "$line" | awk '{print $1}')"
    regex_path="$(echo "$line" | awk '{print $2}')"
    commands="$(echo "$line" | awk '{$1=$2=""; print $0}')"

    echo "${commands}," >> commands.h
    echo "$regex_mime { return $i; }" \
        | sed 's|/|[/]|g; s|-|[-]|g' >>  "$index_mime"
    echo "$regex_path { return $i; }" \
        | sed 's|/|[/]|g; s|-|[-]|g' >>  "$index_path"

    i=$((i+1))
done

echo "};" >> commands.h
printf "* { return -1; } */ \n}\n" >> "$index_mime"
printf "* { return -1; } */ \n}\n" >> "$index_path"

re2c --flex-syntax "$index_mime" -o cindex_mime.c
re2c --flex-syntax "$index_path" -o cindex_path.c
