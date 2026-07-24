#!/bin/sh -e

# shellcheck disable=SC2086

set -e
alias trace_on='set -x'
alias trace_off='{ set +x; } 2>/dev/null'

. ./targets

dir="$(readlink -f "$(dirname "$0")")"

cbase="cbase"

target="${1:-build}"

if ! grep -q "$target" targets; then
    echo "usage: $(basename "$0") <targets>"
    cat targets
    exit 1
fi

cross="$2"

printf "\n${0} ${RED}${1} ${2}$RES\n"
PREFIX="${PREFIX:-/usr/local}"
DESTDIR="${DESTDIR:-/}"

main="main.c"
program="piscou"
exe="bin/$program"
mkdir -p "$(dirname "$exe")"

CFLAGS="$CFLAGS -std=c11"
CFLAGS="$CFLAGS -Wextra -Wall"
CFLAGS="$CFLAGS -Wno-unused-macros -Wno-unused-function"
CFLAGS="$CFLAGS -Wno-unknown-pragmas"
CFLAGS="$CFLAGS -Wfatal-errors"
CFLAGS="$CFLAGS -Wno-gnu-union-cast"
CPPFLAGS="$CPPFLAGS -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700"
CPPFLAGS="$CPPFLAGS -I "$dir/$cbase""
CPPFLAGS="$CPPFLAGS -I "$dir""
LDFLAGS="$LDFLAGS -lmagic -lm"

OS=$(uname -a)

if echo "$OS" | grep -q "Linux"; then
    if echo "$OS" | grep -q "GNU"; then
        GNUSOURCE="-D_GNU_SOURCE"
    fi
fi

option_remove() {
    echo "$1" | sed "s/$2//g"
}

HOST_CC=${HOST_CC:-cc}
CC=${CC:-cc}

if [ ! -d bin ]; then
    mkdir -p bin
fi

case "$target" in
"check")
    CC=gcc CFLAGS="-fanalyzer -fdiagnostics-color=never" "$0" build
    CFLAGS="--analyze -Xanalyzer -analyzer-output=text"
    CFLAGS="$CFLAGS -Xanalyzer -analyzer-werror"
    CFLAGS="$CFLAGS -Xanalyzer -analyzer-opt-analyze-headers"
    CFLAGS="$CFLAGS -Wno-unused-command-line-argument"
    CFLAGS="$CFLAGS -fno-color-diagnostics"
    CC=clang CFLAGS="$CFLAGS" "$0" build
    exit
    ;;
"debug")
    CFLAGS="$CFLAGS -g -fsanitize=undefined"
    CPPFLAGS="$CPPFLAGS $GNUSOURCE -DDEBUGGING=1"
    exe="bin/${program}_debug"
    ;;
"benchmark")
    CFLAGS="$CFLAGS -O2 -flto -march=native -ftree-vectorize"
    CPPFLAGS="$CPPFLAGS $GNUSOURCE -DPISCOU_BENCHMARK=1"
    exe="bin/${program}_benchmark"
    ;;
"perf")
    CFLAGS="$CFLAGS -g3 -Og -flto"
    CPPFLAGS="$CPPFLAGS $GNUSOURCE -DPISCOU_BENCHMARK=1"
    exe="bin/${program}_perf"
    ;;
"valgrind")
    CFLAGS="$CFLAGS -g -O2 -flto -ftree-vectorize"
    CPPFLAGS="$CPPFLAGS $GNUSOURCE -DDEBUGGING=1"
    ;;
"build")
    CFLAGS="$CFLAGS $GNUSOURCE -O2 -flto -march=native -ftree-vectorize"
    ;;
*)
    CFLAGS="$CFLAGS -O2"
    ;;
esac

if [ "$target" = "cross" ]; then
    CC="zig cc"
    CFLAGS="$CFLAGS -target $cross"
    CFLAGS=$(option_remove "$CFLAGS" "-D_GNU_SOURCE")

    case $cross in
    "x86_64-macos"|"aarch64-macos")
        CFLAGS="$CFLAGS -fno-lto"
        LDFLAGS="$LDFLAGS -lpthread"
        ;;
    *windows*)
        exe="bin/$program.exe"
        ;;
    *)
        LDFLAGS="$LDFLAGS -lpthread"
        ;;
    esac
else
    LDFLAGS="$LDFLAGS -lpthread"
fi

if [ "$CC" = "clang" ]; then
    CFLAGS="$CFLAGS -Weverything"
    CFLAGS="$CFLAGS -Wno-unsafe-buffer-usage"
    CFLAGS="$CFLAGS -Wno-format-nonliteral"
    CFLAGS="$CFLAGS -Wno-format-pedantic"
    CFLAGS="$CFLAGS -Wno-pre-c11-compat"
    CFLAGS="$CFLAGS -Wno-disabled-macro-expansion"
    CFLAGS="$CFLAGS -Wno-c++-keyword"
    CFLAGS="$CFLAGS -Wno-c++-compat"
    CFLAGS="$CFLAGS -Wno-covered-switch-default"
    CFLAGS="$CFLAGS -Wno-implicit-void-ptr-cast"
    CFLAGS="$CFLAGS -Wno-cast-qual"
    CFLAGS="$CFLAGS -Wno-constant-logical-operand"
    CFLAGS="$CFLAGS -Wno-cast-function-type-strict"
    CFLAGS="$CFLAGS -Wno-c23-extensions"
    CFLAGS="$CFLAGS -Wno-padded"
    CFLAGS="$CFLAGS -Wno-assign-enum"
    CFLAGS="$CFLAGS -Wno-integer-enum-cast"

    # TODO: implement safe floating point comparisons
    CFLAGS="$CFLAGS -Wno-float-equal"
fi

case "$target" in
"uninstall")
    trace_on
    rm -f ${DESTDIR}${PREFIX}/bin/${program}
    rm -f ${DESTDIR}${PREFIX}/man/man1/${program}.1
    exit
    ;;
"install")
    trace_on
    install -Dm755 bin/${program}   ${DESTDIR}${PREFIX}/bin/${program}
    install -Dm644 ${program}.1 ${DESTDIR}${PREFIX}/man/man1/${program}.1
    exit
    ;;
*)
    trace_on
    ctags --kinds-C=+l+d ./*.h ./*.c meta*.h meta*.c gen/*.h \
        2> /dev/null || true
    vtags.sed tags > .tags.vim       2> /dev/null || true

    mkdir -p gen || true

    PREPROC_CPPFLAGS="$CPPFLAGS"
    case " $PREPROC_CPPFLAGS " in
        *" -DDEBUGGING="*) ;;
        *) PREPROC_CPPFLAGS="$PREPROC_CPPFLAGS -DDEBUGGING=0" ;;
    esac

    $HOST_CC $PREPROC_CPPFLAGS -std=c11 -O2 \
        -o bin/meta_preproc meta_regex/meta_preproc_0_main.c -lm
    ./bin/meta_preproc config.h > gen/config2.h

    $CC $CPPFLAGS $CFLAGS -o ${exe} main.c $LDFLAGS

    trace_off
    ;;
esac
