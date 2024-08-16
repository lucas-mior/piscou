#!/bin/sh

# shellcheck disable=SC2086
testing () {
    for src in *.c; do
        [ "$src" = "main.c" ] && continue
        printf "Testing $src...\n"

        flags="$(awk '/flags:/ { $1=$2=""; print $0 }' "$src")"
        set -x
        if $CC $CFLAGS -D TESTING_THIS_FILE=1 $src -o $src.exe $flags; then
            ./$src.exe
        else
            printf "Failed to compile ${RED} $src ${RES}, is main() defined?\n"
        fi

        set +x 
    done
    rm ./*.exe
}

target="${1:-build}"
PREFIX="${PREFIX:-/usr/local}"
DESTDIR="${DESTDIR:-/}"

main="main.c"
program="piscou"

LDFLAGS="$LDFLAGS $(pkg-config libmagic --libs)"

CC=${CC:-cc}
if [ $CC = "clang" ]; then
    CFLAGS="$CFLAGS -Weverything "
    CFLAGS="$CFLAGS -Wno-unsafe-buffer-usage -Wno-format-nonliteral "
fi
CFLAGS="$CFLAGS -Wextra -Wall "
CFLAGS="$CFLAGS -Wno-disabled-macro-expansion -Wno-unused-parameter "
CFLAGS="$CFLAGS -Wno-unused-variable "

echo "target=$target"
if [ "$target" = "debug" ]; then
    CFLAGS="$CFLAGS -g -fsanitize=undefined "
    CPPFLAGS="$CPPFLAGS -DPISCOU_DEBUG=1"
else
    CFLAGS="$CFLAGS -O2 -flto "
    CPPFLAGS="$CPPFLAGS -DPISCOU_DEBUG=0"
fi

case "$target" in
    "uninstall")
        set -x
        rm -f ${DESTDIR}${PREFIX}/bin/${program}
        rm -f ${DESTDIR}${PREFIX}/man/man1/${program}.1
        ;;
    "test")
        testing
        ;;
    "install")
        [ ! -f $program ] && $0 build
        set -x
        install -Dm755 ${program} ${DESTDIR}${PREFIX}/bin/${program}
        install -Dm644 ${program}.1 ${DESTDIR}${PREFIX}/man/man1/${program}.1
        ;;
    "build"|"debug")
        ctags --kinds-C=+l ./*.h ./*.c 2> /dev/null || true
        vtags.sed tags > .tags.vim 2> /dev/null || true
        set -x
        $CC $CPPFLAGS $CFLAGS -o ${program} "$main" $LDFLAGS
        ;;
    *)
        echo "usage: $0 [ uninstall / test / install / build / debug ]"
        ;;
esac
