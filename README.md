# piscou

File previewer. Piscou executes a program acording to the file's mimetype.

## Installation
```
$ make
# make install
```

## Usage
```
piscou %piscou-filename% [%piscou-extra1 ...]
```

Note that you can pass up to 9 arguments to the program after the filename.

## Configuration
Edit `piscou.h` and recompile.
By default piscou will only call file(1) in order to show the mimetype of the file.
For instance, in order to change the previewer for text,
add the following line:
```
{"text/.+", {"cat", "%piscou-filename%"}}
```

## Rationale
Piscou is intended to be as fast as possible,
so you should probably choose previewing programs that have a short startup time.
