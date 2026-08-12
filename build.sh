#!/bin/sh -e

# shellcheck disable=SC2086

dir=$(dirname "$(readlink -f "$0")")
cd "$dir" || exit

# shellcheck source=./cbase/common.sh
. "./cbase/common.sh"

script=$(basename "$0")

if [ -f ./targets ]; then
    . ./targets
else
    targets=$(cat <<'EOF_TARGETS'
build
debug
fast_feedback
install
uninstall
test
check
benchmark
perf
valgrind
cross x86_64-linux
cross aarch64-linux
cross x86_64-macos
cross aarch64-macos
cross x86_64-windows-gnu
EOF_TARGETS
)
fi

common_build_parse_args "$@"
common_build_validate_mode "$script" "$targets"
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

if [ "$CC" = "clang" ]; then
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
    set +e

    CC=gcc CFLAGS="-fanalyzer -fdiagnostics-color=never" "$0" build

    CFLAGS="--analyze -Xanalyzer -analyzer-output=text"
    CFLAGS="$CFLAGS -Xanalyzer -analyzer-werror"
    CFLAGS="$CFLAGS -Xanalyzer -analyzer-opt-analyze-headers"
    CFLAGS="$CFLAGS -Wno-unused-command-line-argument"
    CFLAGS="$CFLAGS -fno-color-diagnostics"
    CC=clang CFLAGS="$CFLAGS" "$0" build

    exit
    ;;
debug)
    CFLAGS="$CFLAGS -g -fsanitize=undefined"
    CPPFLAGS="$CPPFLAGS -DDEBUGGING=1"
    exe="bin/${program}_debug"
    ;;
benchmark)
    CFLAGS="$CFLAGS -O2 -flto -march=native -ftree-vectorize"
    CPPFLAGS="$CPPFLAGS -DPISCOU_BENCHMARK=1"
    exe="bin/${program}_benchmark"
    ;;
perf)
    CFLAGS="$CFLAGS -g3 -Og -flto"
    CPPFLAGS="$CPPFLAGS -DPISCOU_BENCHMARK=1"
    exe="bin/${program}_perf"
    ;;
valgrind)
    CFLAGS="$CFLAGS -g -O2 -flto -ftree-vectorize"
    CPPFLAGS="$CPPFLAGS -DDEBUGGING=1"
    ;;
build)
    CFLAGS="$CFLAGS -O2 -flto -march=native -ftree-vectorize"
    ;;
*)
    CFLAGS="$CFLAGS -O2"
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
*)
    trace_on
    common_build_tags . gen

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
