# List navigation

**** or **** Move to previous entry
**** or **** Move to next entry

**PgUp**   Move 10 entries up
**PgDown** Move 10 entries down

**Home**   Move to the beginning of list
**End**    Move to the end of list

# Actions

**F1**     Toggle this help screen
**F2**     Loop between title and
       in-programs screen shots
**F3**     Show program info
**F5**     Run configuration program
**F10**    Exit this program
**Enter**  Launch **selected** list entry

# Search & filtering

Type text to automatically filter out
entries that does not match search
string in a case-insensitive manner.

# Lists

You can create lists by making a
directory in .\LISTS\ with the name
you want for list:

.\LISTS\LIST_DIRECTORY
  Ã LIST.TXT  list entries definition
  Ã TITLES\   programs titles screens
  Ã IN_PROGS\ in-programs screen-shots
  À INFO\     programs info files

### LIST.TXT

List of entries, one per line, with
these columns:
- absolute path with drive letter
- executable filename
- setup program filename (if exists)
- number of cycles
- program title

Lines that starts with # character
will be *recognized as comments*.

Each column must be separated by one
or more *white-space characters* (spaces
or tabs).

**Executable filename** is the executable
or batch file to be used to launch the
program. Extension is *mandatory*.

**Setup filename** is the executable or
batch file to be used to setup the
program. When setup program not exists
a - character must be specified.
Extension is *mandatory*.

The **cycles** will be used inside DOSBox
to automatically set correct emulation
speed. If zero is specified 'auto'
emulation speed will be set.

The **program title** will be read
until end-of-line character or
end-of-file, so it can contains any
character.

### TITLES subdirectory

This directory will contains optional
programs titles screen-shots as an
standard *4 bits-per-pixel uncompressed*
*Windows bitmap*. Only first **14 colors**
can be used, 2 colors are reserved by
the user interface of this program.
Width of the image *must be 320 pixels*.

### IN_PROGS subdirectory

This directory will contains optional
in-programs screen-shots with the same
format and size of title screen-shots.

### INFO subdirectory

This directory will contains optional
simplified markdown files what will
describe specific program usage,
options, keyboard shortcuts or notes,
with a **maximum line length of 38**
**characters**, markdown formatting
excluded.


**NOTE:** titles, in-programs and info
filenames *must match the inner*
*directory name* of the absolute path
specified in LIST.TXT file.
