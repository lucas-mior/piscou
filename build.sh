#!/bin/sh

# shellcheck disable=SC2086

alias trace_on='set -x'
alias trace_off='{ set +x; } 2>/dev/null'

target="${1:-build}"
PREFIX="${PREFIX:-/usr/local}"
DESTDIR="${DESTDIR:-/}"

main="main.c"
program="piscou"

LDFLAGS="$LDFLAGS $(pkg-config libmagic --libs)"

CC=${CC:-cc}

CFLAGS="$CFLAGS -std=c99 -D_DEFAULT_SOURCE"
CFLAGS="$CFLAGS -Wextra -Wall"
CFLAGS="$CFLAGS -Wno-constant-logical-operand"
CFLAGS="$CFLAGS -Wno-unused-function"
CFLAGS="$CFLAGS -Wno-unknown-pragmas"
if [ $CC = "clang" ]; then
    CFLAGS="$CFLAGS -Weverything"
    CFLAGS="$CFLAGS -Wno-unsafe-buffer-usage"
    CFLAGS="$CFLAGS -Wno-format-nonliteral"
    CFLAGS="$CFLAGS -Wno-implicit-void-ptr-cast"
fi

echo "$0 $target ..."
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
    trace_on
    rm -f ${DESTDIR}${PREFIX}/bin/${program}
    rm -f ${DESTDIR}${PREFIX}/man/man1/${program}.1
    ;;
"install")
    if [ ! -f $program ]; then
        $0 build
    fi
    trace_on
    install -Dm755 ${program} ${DESTDIR}${PREFIX}/bin/${program}
    install -Dm644 ${program}.1 ${DESTDIR}${PREFIX}/man/man1/${program}.1
    ;;
"build"|"debug"|"benchmark")
    trace_on
    ctags --kinds-C=+l ./*.h ./*.c 2> /dev/null || true
    vtags.sed tags > .tags.vim     2> /dev/null || true
    $CC $CPPFLAGS $CFLAGS -o ${program} "$main" $LDFLAGS
    ;;
*)
    echo "usage: $0 [ uninstall / install / build / debug ]"
    ;;
esac
