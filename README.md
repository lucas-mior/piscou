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
You have to configure a program and arguments for each regex defined in the `rules` array.
For instance, in order to change the previewer for text,
add the following line:
```
{"text/.+", {"cat", "%piscou-filename%"}}
```
If a line is not matched but the 1st word is exactly `fpath`,
then the 2nd argument is interpreted as a file path regex.

## Rationale
Piscou is intended to be as fast as possible,
so you should probably choose previewing programs that have a short startup time.
