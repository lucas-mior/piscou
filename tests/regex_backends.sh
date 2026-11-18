#!/bin/sh -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmp_dir=$(mktemp -d)
trap 'rm -rf "$tmp_dir"' EXIT
trap 'exit 1' HUP INT TERM

if [ ! -d "$root/meta_regex/src" ]; then
    printf 'This comparison requires meta_regex sources.\n' >&2
    exit 1
fi

compiler=${CC:-cc}

fail() {
    printf 'Regex backend test failed: %s\n' "$1" >&2
    exit 1
}

contains() {
    if ! grep -Fq -- "$2" "$1"; then
        cat "$1" >&2
        fail "missing expected text: $2"
    fi
}

for backend in meta libc; do
    workspace="$tmp_dir/$backend/piscou"
    mkdir -p "$workspace"
    cp "$root/build.sh" "$root/main.c" "$root/config.h" \
        "$root/piscou.1" "$root/piscou_regex.h" "$root/piscou_regex.c" \
        "$workspace/"
    ln -s "$root/cbase" "$workspace/cbase"
    if [ "$backend" = meta ]; then
        ln -s "$root/meta_regex" "$workspace/meta_regex"
        backend_value=0
    else
        backend_value=1
    fi

    (
        cd "$workspace"
        PISCOU_REGEX_BACKEND=auto ./build.sh debug \
            > "$tmp_dir/$backend.build.log" 2>&1
    ) || {
        cat "$tmp_dir/$backend.build.log" >&2
        fail "$backend debug build"
    }
    contains "$tmp_dir/$backend.build.log" "Regex backend: $backend"
    if [ "$backend" = libc ] && [ -e "$workspace/gen/config2.h" ]; then
        fail 'libc build unexpectedly generated config2.h'
    fi

    "$compiler" -std=c11 -w -pthread -I"$workspace" \
        -I"$workspace/cbase" -DPISCOU_REGEX_BACKEND_LIBC="$backend_value" \
        -o "$tmp_dir/$backend.parity" "$root/tests/regex_parity.c" -lm
    "$tmp_dir/$backend.parity" > "$tmp_dir/$backend.parity.out"
done

diff -u "$tmp_dir/meta.parity.out" "$tmp_dir/libc.parity.out" ||
    fail 'rule matching or capture offsets differ'
contains "$tmp_dir/libc.parity.out" '45 16 1 1'
contains "$tmp_dir/libc.parity.out" '45 17 0 1'
contains "$tmp_dir/libc.parity.out" '45 18 1 0'
contains "$tmp_dir/libc.parity.out" 'direct 0 1 0 10 8 9 -1 -1'
contains "$tmp_dir/libc.parity.out" 'embedded 2 1 6 16 14 15 -1 -1'
contains "$tmp_dir/libc.parity.out" 'direct 5 0 -1 -1 -1 -1 -1 -1'

printf 'name,value\na,1\n' > "$tmp_dir/table.csv"
printf 'hello\n' > "$tmp_dir/readme.txt"
printf '{}\n' > "$tmp_dir/sample.json"
printf 'GIF89a' > "$tmp_dir/picture.gif"
mkdir -p "$tmp_dir/folder"

for scenario in csv missing text json gif directory nonexistent; do
    for backend in meta libc; do
        binary="$tmp_dir/$backend/piscou/bin/piscou"
        case "$scenario" in
        csv)
            set -- "$tmp_dir/table.csv" 80 24 one two
            ;;
        missing)
            set -- "$tmp_dir/table.csv" 80
            ;;
        text)
            set -- "$tmp_dir/readme.txt"
            ;;
        json)
            set -- "$tmp_dir/sample.json"
            ;;
        gif)
            set -- "$tmp_dir/picture.gif" 80 24 one two
            ;;
        directory)
            set -- "$tmp_dir/folder"
            ;;
        nonexistent)
            set -- "$tmp_dir/missing.fen"
            ;;
        esac
        if "$binary" "$@" > "$tmp_dir/$backend.$scenario.out" 2>&1; then
            fail "$backend $scenario: expected debug-mode failure exit"
        fi
    done
    diff -u "$tmp_dir/meta.$scenario.out" \
        "$tmp_dir/libc.$scenario.out" || fail "$scenario output mismatch"
done

contains "$tmp_dir/libc.csv.out" 'args.array[0] = csv.sh'
contains "$tmp_dir/libc.missing.out" 'Extra argument 1 not passed'
contains "$tmp_dir/libc.gif.out" 'args.array[2] = --size=80x24'
contains "$tmp_dir/libc.gif.out" 'args.array[0] = stiv_draw'
contains "$tmp_dir/libc.gif.out" 'args.array[0] = chafa'
contains "$tmp_dir/libc.directory.out" 'args.array[0] = ls'

for backend in meta libc; do
    workspace="$tmp_dir/$backend/piscou"
    (
        cd "$workspace"
        PISCOU_REGEX_BACKEND=auto ./build.sh fast_feedback \
            > "$tmp_dir/$backend.release.log" 2>&1
    ) || {
        cat "$tmp_dir/$backend.release.log" >&2
        fail "$backend release build"
    }
done

mkdir -p "$tmp_dir/only_chafa" "$tmp_dir/with_both"
cat > "$tmp_dir/only_chafa/chafa" <<'MOCK'
#!/bin/sh
printf 'chafa'
for argument do
    printf ' [%s]' "$argument"
done
printf '\n'
MOCK
cat > "$tmp_dir/with_both/stiv_draw" <<'MOCK'
#!/bin/sh
printf 'stiv_draw'
for argument do
    printf ' [%s]' "$argument"
done
printf '\n'
exit 7
MOCK
chmod +x "$tmp_dir/only_chafa/chafa" "$tmp_dir/with_both/stiv_draw"
cp "$tmp_dir/only_chafa/chafa" "$tmp_dir/with_both/chafa"

for backend in meta libc; do
    binary="$tmp_dir/$backend/piscou/bin/piscou"
    PATH="$tmp_dir/only_chafa" "$binary" "$tmp_dir/picture.gif" 80 24 \
        > "$tmp_dir/$backend.fallback.out" 2>&1 ||
        fail "$backend fallback execution"
    if PATH="$tmp_dir/with_both" "$binary" "$tmp_dir/picture.gif" 80 24 \
        > "$tmp_dir/$backend.ordered.out" 2>&1; then
        fail "$backend expected stub exit 7"
    else
        status=$?
        [ "$status" -eq 7 ] || fail "$backend stub exit code: $status"
    fi
done

diff -u "$tmp_dir/meta.fallback.out" "$tmp_dir/libc.fallback.out" ||
    fail 'fallback execution differs'
diff -u "$tmp_dir/meta.ordered.out" "$tmp_dir/libc.ordered.out" ||
    fail 'first executable ordering differs'
contains "$tmp_dir/libc.fallback.out" "chafa [$tmp_dir/picture.gif] [--size=80x24]"
contains "$tmp_dir/libc.ordered.out" "stiv_draw [$tmp_dir/picture.gif] [80] [24]"

"$compiler" -std=c11 -w -pthread -I"$root" -I"$root/cbase" \
    -DPISCOU_REGEX_BACKEND_LIBC=1 -o "$tmp_dir/libc.invalid" \
    "$root/tests/regex_invalid.c" -lm
if "$tmp_dir/libc.invalid" > "$tmp_dir/invalid.out" 2>&1; then
    fail 'invalid POSIX pattern was accepted'
fi
contains "$tmp_dir/invalid.out" 'Invalid POSIX regex'

"$compiler" -std=c11 -w -pthread -I"$root" -I"$root/cbase" \
    -DPISCOU_REGEX_BACKEND_LIBC=1 -o "$tmp_dir/libc.unit" \
    "$root/piscou_regex.c" -lm
"$tmp_dir/libc.unit" || fail 'POSIX standalone tests'

printf 'Both regex backends passed matching, captures, arguments, and fallback.\n'
