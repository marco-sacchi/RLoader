# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [1.0.2] - 2021-08-14

### Changed
- rewritten the resident executable in assembly to minimize memory occupation
  and the allow to load it in upper memory.

## [1.0.1] - 2021-07-17

### Added
- maintained user preference between `title` and `in_progs` thumbnails
- made persistent the currently selected entry and the thumbnail display
  preference between RLoader sessions
- added arguments `/batchpause` and `/batchmemfree` to simplify
  troubleshooting when launching programs
- added check to verify that thumbnail bitmaps are uncompressed at
  4-bits-per-pixel.
