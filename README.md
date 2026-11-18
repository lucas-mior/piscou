# piscou
File previewer.
piscou executes a program according to a file's MIME type and/or filename.

## Installation
A C11 compiler, `pkg-config`, and the development files for `libmagic` are
required. The POSIX `<regex.h>` interface is used when `meta_regex` is absent;
`meta_regex` itself is optional.

```sh
git clone https://github.com/lucas-mior/piscou
cd piscou
./build.sh
sudo ./build.sh install
```

### Regex backends
`build.sh` selects the regex backend through the `PISCOU_REGEX_BACKEND`
environment variable:

| Value | Selection |
| --- | --- |
| `auto` (default) | Use `meta_regex` if `meta_regex/` exists; otherwise use libc. |
| `meta` | Require a complete `meta_regex/` source tree. |
| `libc` | Use POSIX `<regex.h>` even if `meta_regex/` exists. |

To override automatic selection for a build:

```sh
PISCOU_REGEX_BACKEND=libc ./build.sh
PISCOU_REGEX_BACKEND=meta ./build.sh
```

The optional `meta_regex` sources must be under `meta_regex/` at the project
root, with their headers and preprocessor sources under `meta_regex/src/`.
If that directory exists but is incomplete, `auto` and `meta` report a build
error; use `libc` to build without it. An invalid backend name also causes a
build error.

The `meta` backend builds `bin/meta_preproc` and generates `gen/config2.h`
from `config.h`. The `libc` backend includes `config.h` directly: it neither
uses a generated configuration header nor runs the `meta_regex` preprocessor.
No `meta_regex` sources are required for this build.

## Usage
```text
piscou #piscou-file# [#piscou-0# #piscou-1# ...]
```

You can pass up to 10 extra arguments after the filename with the default
`MAX_EXTRAS` setting in `config.h` (referenced as `#piscou-0#` through
`#piscou-9#`).

## Configuration
Edit `config.h` and recompile. By default, piscou might call programs you
have not installed.

Rules match a MIME type, filename, or both. At least one matching field
must be set; use `NULL` for an unconstrained field. Write patterns with the
same `R("pattern")` macro for either regex backend. Existing rule definitions
do not need to change when switching
backends. The two-argument `R("pattern", flags)` form is also supported;
`META_RE_NONE`, `META_RE_YESSUB`, and `META_RE_NOSUB` are available in both
modes. The `R(...)` macro can be used in static rule initializers.

### Regex compatibility
In libc mode, patterns use POSIX extended regular expressions, compiled with
`REG_EXTENDED`. The `meta_regex` backend uses its own matching engine and
preprocessor. Prefer the common POSIX ERE subset when a configuration needs
to work with both; PCRE-specific syntax is not supported by the libc backend.
Matching and capture-group behavior may differ for patterns outside the common
subset, including locale-sensitive expressions. Test custom patterns with
both backends if identical matching is important.

The `#piscou-N#` placeholders in command arguments are independent of the
regex backend. Piscou substitutes extra arguments into direct placeholders
and embedded text, such as `--size=#piscou-0#x#piscou-1#`.

The regression suite compares both backends when `meta_regex` is available:

```sh
./tests/regex_backends.sh
```

This suite requires the optional `meta_regex/` source tree, even though a
normal libc-only build does not.

### Limitations
All of those are configurable via `config.h`.
The sane defaults are:
- You can pass up to 10 "extra" arguments to piscou.
- You can pass up to 16 arguments to each program.
- Each argument of those 16 can be up to 256 characters long.

## Rationale
piscou is intended to be as fast as possible, so you should probably choose
previewing programs that have a short startup time.
