# RLoader

DOS programs launcher with customizable lists, search and thumbnails
display.

Developed entirely in Borland C++ 3.0 as a simple list viewer to launch DOS
programs, it has been extended by integrating:
- text search,
- definition of multiple program lists,
- display of thumbnails of the program title and in-program graphics,
- simplified display of markdown files as help and information on the
  elements of the active list
- a fully functional text-mode interface for low-end machines

The interface designed for the arrow keys and only 5 other function keys
makes it perfect to use in DOSBox even when user input is handled via a
gamepad (and proper key mapping).

![SCREEN01](RES/SCREEN01.PNG?raw=true "SCREEN01")
![SCREEN02](RES/SCREEN02.PNG?raw=true "SCREEN02")
![SCREEN03](RES/SCREEN03.PNG?raw=true "SCREEN03")
![SCREEN04](RES/SCREEN04.PNG?raw=true "SCREEN04")
![SCREEN05](RES/SCREEN05.PNG?raw=true "SCREEN05")

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

RLoader was compiled using the 8086 instruction set and implements two user
interfaces: one graphical and one in text mode.

The former requires a compatible VGA graphics card while the latter makes it
usable even on low-end machines with a compatible CGA graphics card.

Thanks to [jsmolina](https://github.com/jsmolina) for pointing out a
compatibility issue in some emulators when text mode is used on CGA graphics
cards and to [mills32](https://github.com/mills32) for testing on
[pcem](https://pcem-emulator.co.uk) and [dosbox-x](https://dosbox-x.com).

Now RLoader works correctly even on CGA mono and composite, tandy, pcjr,
amstrad, mcga, mda machines.

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

If you want to use the text-mode interface, type:

```
RLOADER /textmode example
```

## Running from any location

`RLOADER.EXE` is designed to search for `UI.EXE` in the current working
directory, creating some problems in some particular execution scenarios,
like:

```
E:\UTILS\RLOADER.EXE example
```

Since version 1.0.3 it is possible to define the environment variable
`RLOADER_PATH` to specify the path of rloader, so that it can be executed from
any location.

The variable can contain the drive designator, it is treated in a 
case-insensitive way and **must not end** with the directory separator
character (`\` or `/`). Example of valid paths:

```
set RLOADER_PATH=D:\UTILITIES\RLOADER
set RLOADER_PATH=c:\rloader
```

# Loading RLoader in upper memory

As pointed out by [cyberluke](https://github.com/cyberluke) the resident part
of RLoader occupies an entire segment (64Kb) even if the executable is smaller
in size.

Now that it has been rewritten in assembly, a TSR-like behavior has been
implemented whereby all unused memory paragraphs are released immediately
after loading. This, together with the reduction of a tenth of the file size,
makes it easier to load into upper memory, leaving the rest of the
conventional memory available to the launched programs.

```
LH RLOADER example
```

# Troubleshooting

RLoader also has arguments to simplify troubleshooting if the launch of the
listed programs fails:

- `/batchpause` pauses the dynamically generated batch file that launches the
  program just before returning to RLoader, in order to make any error
  messages readable;

- `/batchmemfree` displays the memory available to the program that is
  launched using the `mem /free` command, available both in compatible MS-DOS
  environments and in the various flavors of DOSBox.

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

### Thumbnails generation

rloader thumbnails must be standard *4 bits-per-pixel uncompressed Windows*
*bitmap*. Only first **14 colors** can be used, 2 colors are reserved by the
user interface of this program. Width of the image *must be 320 pixels*.

You can use any command line tool that supports image manipulation. For 
example using [ImageMagick](https://imagemagick.org/), which is a free and
open source tool, you can write:

```
magick.exe input.png -resize 320 -dither Ordered -depth 4 -colors 14 -type palette bmp3:output.bmp
```

where:
- `-resize 320` rescales the image to 320 pixels
- `-dither Ordered` applies ordered dithering (optional)
- `-depth 4` specifies 4 bits per pixel (16 colors)
- `-colors 14` remaps using only 14 colors
- `type color` ensures conversion to a palettized image
- `bmp3` specifies an uncompressed Windows bitmap

You can avoid bilinear filtering caused by downscaling by adding the
`-filter` flag with the values ​​`Point` or `NearestNeighbor`.

### TITLES subdirectory

This directory will contains optional programs titles screen-shots thumbnails.

### IN_PROGS subdirectory

This directory will contains optional in-programs screen-shots thumbnails.

### INFO subdirectory

This directory will contains optional simplified markdown files what will
describe specific program usage, options, keyboard shortcuts or notes,
with a **maximum line length of 38 characters**, markdown formatting excluded.


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
