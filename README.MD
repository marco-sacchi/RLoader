# RLoader

DOS programs launcher with customizable lists, search and thumbnails
display.

Developed entirely in Borland C ++ 3.0 as a simple list viewer to launch DOS
programs, it has been extended by integrating:
- text search,
- definition of multiple program lists,
- display of thumbnails of the program title and in-program graphics,
- simplified display of markdown files as help and information on the
  elements of the active list.

The interface designed for the arrow keys and only 5 other function keys
makes it perfect to use in DOSBox even when user input is handled via a
gamepad (and proper key mapping).

![SCREEN01](RES/SCREEN01.PNG?raw=true "SCREEN01")
![SCREEN02](RES/SCREEN02.PNG?raw=true "SCREEN02")
![SCREEN03](RES/SCREEN03.PNG?raw=true "SCREEN03")

# Installation

Simply copy the two executable files and the `res` and` lists` folders to
a folder of your choice.

# Configuration

RLoader is configurable through the `RES/RLOADER.CFG` file. 

The foreground and background colors of the user interface and the mapping
of the drives specified in the lists can be customized (see the comments in
the configuration file for more info).

This last feature allows the porting of the lists from one system to another
without the need to modify the absolute paths.

# Hardware requirements (for a real machine)

An 80386 33 MHz with VGA-compatible graphics card.

# Execution

You can run RLoader without arguments or with `/h` or `/?` to view a short
help.

To load the example list on real hardware or VM, type:

```
RLOADER example
```

if you are on DOSBox, type:

```
RLOADER /dosbox example
```

# List and info navigation

- **Arrow keys** Move to previous/next entry.
- **PgUp**/**PgDown** Move ten lines up/down.
- **Home**/**End** Move to the beginning/end of list.

# Actions

- **F1** Toggle this help screen
- **F2** Loop between title and in-programs screen shots
- **F3** Show program info
- **F5** Run configuration program
- **F10** Exit this program
- **Enter**  Launch selected list entry

# Search & filtering

Type text to automatically filter out entries that does not match search
string in a case-insensitive manner.

# Lists

You can create lists by making a directory in `.\LISTS\` with the name
you want for list:

```
.\LISTS\LIST_DIRECTORY
    LIST.TXT  list entries definition
    TITLES\   programs titles screens
    IN_PROGS\ in-programs screen-shots
    INFO\     programs info files
```

### LIST.TXT

List of entries, one per line, with
these columns:
- absolute path with drive letter
- executable filename
- setup program filename (if exists)
- number of cycles
- program title

Lines that starts with **#** character will be *recognized as comments*.

Each column must be separated by one or more *white-space characters*
(spaces or tabs).

**Executable filename** is the executable or batch file to be used to launch
the program. Extension is *mandatory*.

**Setup filename** is the executable or batch file to be used to setup the
program. When setup program not exists a - character must be specified.
Extension is *mandatory*.

The **cycles** will be used inside DOSBox to automatically set correct
emulation speed. If zero is specified 'auto' emulation speed will be set.

The **program title** will be read until end-of-line character or
end-of-file, so it can contains any character.

### TITLES subdirectory

This directory will contains optional programs titles screen-shots as an
standard *4 bits-per-pixel uncompressed* *Windows bitmap*. Only first 
**14 colors** can be used, 2 colors are reserved by the user interface of
this program. Width of the image *must be 320 pixels*.

### IN_PROGS subdirectory

This directory will contains optional in-programs screen-shots with the same
format and size of title screen-shots.

### INFO subdirectory

This directory will contains optional simplified markdown files what will
describe specific program usage, options, keyboard shortcuts or notes,
with a **maximum line length of 38** **characters**, markdown formatting
excluded.


**NOTE:** titles, in-programs and info filenames *must match the inner*
*directory name* of the absolute path specified in LIST.TXT file.

# How to compile

You need the Borland C++ 3.0 development environment to be able to compile
the program from source. At the moment the code is not fully compatible with
other compilers, so executables are included.

There are two projects, `UI.PRJ` and `RLOADER/RLOADER.PRJ`, both of which
can be opened in the IDE and compiled. The `RLOADER.EXE` executable needs to
be copied to the root folder and that's what needs to be run.

# License

MIT License, Copyright (c) 2021 Marco Sacchi
