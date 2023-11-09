# piscou
File previewer.
piscou executes a program acording to the file's mimetype and/or filename.

## Installation
```
$ git clone https://github.com/lucas-mior/piscou
$ cd piscou
$ make
$ sudo make install
```

## Usage
```
piscou %piscou-filename% [%piscou-extra0% %piscou-extra1% ...]
```

Note that you can pass up to 9 arguments to the program after the filename.

## Configuration
Edit `config.h` and recompile.
Note that by default, piscou might call programs you don't have installed.

### Limitations
All of those are configurable via `config.h`.
The sane defaults are:
- You can pass up to 10 "extra" arguments to piscou.
- You can pass up to 16 arguments to each program.
- Each argument of those 16 can be up to 256 characters long.

## Rationale
piscou is intended to be as fast as possible, so you should probably choose
previewing programs that have a short startup time.
