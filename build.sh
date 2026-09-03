#!/bin/sh -e

# shellcheck disable=SC2086

dir=$(dirname "$(readlink -f "$0")")
cd "$dir" || exit

# shellcheck source=./cbase/common.sh
. "./cbase/common.sh"

script=$(basename "$0")


common_build_parse_args "$@"

case "$mode" in
benchmark|build|check|cross|debug|debug-fast|fast_feedback|install|perf|test|uninstall|valgrind)
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

OS=$(uname -a)
if [ "$mode" = "cross" ]; then
    if [ "$target" != "all" ]; then
        OS="$target"
    fi
fi

PTHREAD_CFLAGS=
case "$OS" in
*MINGW*|*MSYS*|*CYGWIN*|*mingw*|*msys*|*cygwin*|*windows*)
    ;;
*)
    PTHREAD_CFLAGS="-pthread"
    ;;
esac

CPPFLAGS="$CPPFLAGS -I."
CPPFLAGS="$CPPFLAGS -Icbase"

CFLAGS="$CFLAGS -std=c11"
CFLAGS="$CFLAGS -Wfatal-errors"
CFLAGS="$CFLAGS $PTHREAD_CFLAGS"

if [ "$CC" = "clang" ] || [ "$CC" = "zig cc" ]; then
    CFLAGS="$CFLAGS -Wno-c++-compat"
    CFLAGS="$CFLAGS -Wno-declaration-after-statement"
fi

PKG_CONFIG=${PKG_CONFIG:-pkg-config}
LIBMAGIC_CFLAGS=$($PKG_CONFIG --cflags libmagic)
LIBMAGIC_LDFLAGS=$($PKG_CONFIG --libs libmagic)
LIBMAGIC_RPATH=
case "$(uname -s)" in
NetBSD)
    LIBMAGIC_LIBDIR=$($PKG_CONFIG --variable=libdir libmagic)
    if [ -n "$LIBMAGIC_LIBDIR" ]; then
        LIBMAGIC_RPATH="-Wl,-R$LIBMAGIC_LIBDIR"
    fi
    ;;
*)
    ;;
esac

CPPFLAGS="$CPPFLAGS $LIBMAGIC_CFLAGS"
LDFLAGS="$LDFLAGS $LIBMAGIC_LDFLAGS $LIBMAGIC_RPATH -lm"

HOST_CC=${HOST_CC:-cc}

if [ ! -d bin ]; then
    mkdir -p bin
fi

case "$mode" in
check)
    (
        common_build_run_analyzers build
    )
    echo "static analysis finished."
    exit
    ;;
debug)
    CFLAGS="$CFLAGS -g3"
    CPPFLAGS="$CPPFLAGS -DDEBUGGING=1"
    exe="bin/$program"
    ;;
debug-fast)
    CFLAGS="$CFLAGS -g2 -O2 -flto -march=native -ftree-vectorize"
    CFLAGS="$CFLAGS -fsanitize=undefined"
    CPPFLAGS="$CPPFLAGS -DDEBUGGING=1"
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
benchmark|build|check|cross|debug|debug-fast|fast_feedback|install|perf|test|uninstall|valgrind)
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
        ;;
    *windows*)
        exe="bin/$program.exe"
        ;;
    *)
        ;;
    esac
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
benchmark|build|cross|debug|debug-fast|fast_feedback|perf|valgrind)
    common_build_tags . gen

    mkdir -p gen || true

    PREPROC_CPPFLAGS="$CPPFLAGS"
    case " $PREPROC_CPPFLAGS " in
        *" -DDEBUGGING="*) ;;
        *) PREPROC_CPPFLAGS="$PREPROC_CPPFLAGS -DDEBUGGING=0" ;;
    esac

    trace_on

    $HOST_CC $PREPROC_CPPFLAGS -std=c11 -O2 $PTHREAD_CFLAGS \
        -o bin/meta_preproc meta_regex/src/meta_preproc_0_main.c -lm
    ./bin/meta_preproc config.h > gen/config2.h

    $CC $CPPFLAGS $CFLAGS -o ${exe} main.c $LDFLAGS

    trace_off
    ;;
esac
