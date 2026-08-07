#!/bin/sh -e

# shellcheck disable=SC2086

set -e

compiler=${1:-cc}
shift || true

dir=$(dirname "$(readlink -f "$0")")

compiler_object_suffix () {
    compiler_suffix=$(printf "%s" "$1"         | sed -E 's|.*/||; s|[^[:alnum:]_]+|_|g; s|^_+||; s|_+$||')

    if [ -z "$compiler_suffix" ]; then
        compiler_suffix=cc
    fi

    printf "%s" "$compiler_suffix"
}

object="$dir/cbase-$(compiler_object_suffix "$compiler").o"
cmdfile="$object.cmd"
header="$dir/cbase.h"
cmdline="$compiler $* -DCBASE_IMPLEMENT=1 -x c -c $header -o $object"

object_stale () {
    if [ ! -f "$object" ]; then
        return 0
    fi

    if [ "$dir/pre-compile.sh" -nt "$object" ]; then
        return 0
    fi

    if ! printf "%s
" "$cmdline" | cmp -s "$cmdfile" -; then
        return 0
    fi

    find "$dir" -iname "*.[ch]" -newer "$object" -print -quit | grep -q .
}

if object_stale; then
    $compiler "$@" -DCBASE_IMPLEMENT=1 -x c -c "$header" -o "$object"
    printf "%s
" "$cmdline" > "$cmdfile"
fi

printf "%s
" "$object"
