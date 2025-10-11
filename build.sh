#!/bin/sh

# shellcheck disable=SC2086

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
CFLAGS="$CFLAGS -std=c99 -D_DEFAULT_SOURCE"
CFLAGS="$CFLAGS -Wextra -Wall "
CFLAGS="$CFLAGS -Wno-disabled-macro-expansion -Wno-unused-parameter "
CFLAGS="$CFLAGS -Wno-unused-variable -Wno-unused-function"
CFLAGS="$CFLAGS -Wno-c11-extensions -Wno-constant-logical-operand"

echo "target=$target"
case "$target" in
    "debug")
        CFLAGS="$CFLAGS -g -fsanitize=undefined "
        CPPFLAGS="$CPPFLAGS -DPISCOU_DEBUG=1" ;;
    "benchmark")
        CFLAGS="$CFLAGS -g -O2 -flto "
        CPPFLAGS="$CPPFLAGS -DPISCOU_BENCHMARK=1" ;;
    *)
        CFLAGS="$CFLAGS -O2 -flto "
        CPPFLAGS="$CPPFLAGS -DPISCOU_DEBUG=0" ;;
esac

case "$target" in
    "uninstall")
        set -x
        rm -f ${DESTDIR}${PREFIX}/bin/${program}
        rm -f ${DESTDIR}${PREFIX}/man/man1/${program}.1
        ;;
    "install")
        [ ! -f $program ] && $0 build
        set -x
        install -Dm755 ${program} ${DESTDIR}${PREFIX}/bin/${program}
        install -Dm644 ${program}.1 ${DESTDIR}${PREFIX}/man/man1/${program}.1
        ;;
    "build"|"debug"|"benchmark")
        ctags --kinds-C=+l ./*.h ./*.c 2> /dev/null || true
        vtags.sed tags > .tags.vim 2> /dev/null || true
        set -x
        $CC $CPPFLAGS $CFLAGS -o ${program} "$main" $LDFLAGS
        ;;
    *)
        echo "usage: $0 [ uninstall / install / build / debug ]"
        ;;
esac
