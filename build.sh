#!/bin/sh -e

# shellcheck disable=SC2086

dir=$(dirname "$(readlink -f "$0")")
cd "$dir" || exit

# shellcheck source=./cbase/common.sh
. "./cbase/common.sh"

script=$(basename "$0")


common_build_parse_args "$@"

case "$mode" in
benchmark|build|check|cross|debug|fast_feedback|install|perf|test|uninstall|valgrind)
    ;;
*)
    common_build_unknown_mode
    ;;
esac
cross="$target"

common_build_print_invocation "$script"
PREFIX="${PREFIX:-/usr/local}"
DESTDIR="${DESTDIR:-/}"

program=$(common_get_program "$0")
exe="bin/$program"
mkdir -p "$(dirname "$exe")"

CC=$(common_get_compiler "$mode")

CPPFLAGS="$CPPFLAGS -I."
CPPFLAGS="$CPPFLAGS -Icbase"

CFLAGS="$CFLAGS -std=c11"
CFLAGS="$CFLAGS -Wfatal-errors"
CFLAGS="$CFLAGS -Wextra -Wall"
CFLAGS="$CFLAGS -Werror=all -Werror=extra"
CFLAGS="$CFLAGS -Werror"  # Only uncomment occasionally, keep this line
CFLAGS="$CFLAGS -Wno-unused-function"

if [ "$CC" = "clang" ] || [ "$CC" = "zig cc" ]; then
    CFLAGS="$CFLAGS -Weverything"
    CFLAGS="$CFLAGS -Wno-assign-enum"
    CFLAGS="$CFLAGS -Wno-c++-compat"
    CFLAGS="$CFLAGS -Wno-c++-keyword"
    CFLAGS="$CFLAGS -Wno-cast-qual"
    CFLAGS="$CFLAGS -Wno-constant-logical-operand"
    CFLAGS="$CFLAGS -Wno-covered-switch-default"
    CFLAGS="$CFLAGS -Wno-declaration-after-statement"
    CFLAGS="$CFLAGS -Wno-disabled-macro-expansion"
    CFLAGS="$CFLAGS -Wno-float-equal"
    CFLAGS="$CFLAGS -Wno-format-nonliteral"
    CFLAGS="$CFLAGS -Wno-implicit-int-enum-cast"
    CFLAGS="$CFLAGS -Wno-implicit-void-ptr-cast"
    CFLAGS="$CFLAGS -Wno-padded"
    CFLAGS="$CFLAGS -Wno-pre-c11-compat"
    CFLAGS="$CFLAGS -Wno-unsafe-buffer-usage"
    CFLAGS="$CFLAGS -Wno-unused-macros"
    CFLAGS="$CFLAGS -Wno-used-but-marked-unused"
fi

LDFLAGS="$LDFLAGS -lmagic -lm"

HOST_CC=${HOST_CC:-cc}

if [ ! -d bin ]; then
    mkdir -p bin
fi

case "$mode" in
check)
    common_build_run_analyzers build
    ;;
debug)
    CFLAGS="$CFLAGS -g3"
    CPPFLAGS="$CPPFLAGS -DDEBUGGING=1"
    exe="bin/$program"
    ;;
benchmark)
    CFLAGS="$CFLAGS -O2 -flto -march=native -ftree-vectorize"
    CPPFLAGS="$CPPFLAGS -DPISCOU_BENCHMARK=1"
    exe="bin/$program"
    ;;
perf)
    CFLAGS="$CFLAGS -g3 -Og -flto"
    CPPFLAGS="$CPPFLAGS -DPISCOU_BENCHMARK=1"
    exe="bin/$program"
    ;;
valgrind)
    CFLAGS="$CFLAGS -g -O2 -flto -ftree-vectorize"
    CPPFLAGS="$CPPFLAGS -DDEBUGGING=1"
    ;;
build)
    CFLAGS="$CFLAGS -O2 -flto -march=native -ftree-vectorize"
    ;;
cross)
    common_build_cross_all
    CFLAGS="$CFLAGS -O2"
    ;;
fast_feedback)
    CFLAGS="$CFLAGS -O2"
    ;;
benchmark|build|check|cross|debug|fast_feedback|install|perf|test|uninstall|valgrind)
    ;;
*)
    common_build_unknown_mode
    ;;
esac

if [ "$mode" = "cross" ]; then
    CC="zig cc"
    CFLAGS="$CFLAGS -target $cross"

    case $cross in
    x86_64-macos|aarch64-macos)
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

case "$mode" in
test)
    TEST_EXCLUDE_PATTERN='(^|/)cbase/' common_test "$target"
    exit
    ;;
uninstall)
    trace_on
    rm -f ${DESTDIR}${PREFIX}/bin/${program}
    rm -f ${DESTDIR}${PREFIX}/man/man1/${program}.1
    exit
    ;;
install)
    trace_on
    install -Dm755 bin/${program}   ${DESTDIR}${PREFIX}/bin/${program}
    install -Dm644 ${program}.1 ${DESTDIR}${PREFIX}/man/man1/${program}.1
    exit
    ;;
benchmark|build|cross|debug|fast_feedback|perf|valgrind)
    common_build_tags . gen

    mkdir -p gen || true

    PREPROC_CPPFLAGS="$CPPFLAGS"
    case " $PREPROC_CPPFLAGS " in
        *" -DDEBUGGING="*) ;;
        *) PREPROC_CPPFLAGS="$PREPROC_CPPFLAGS -DDEBUGGING=0" ;;
    esac

    trace_on

    $HOST_CC $PREPROC_CPPFLAGS -std=c11 -O2 \
        -o bin/meta_preproc meta_regex/meta_preproc_0_main.c -lm
    ./bin/meta_preproc config.h > gen/config2.h

    $CC $CPPFLAGS $CFLAGS -o ${exe} main.c $LDFLAGS

    trace_off
    ;;
esac
