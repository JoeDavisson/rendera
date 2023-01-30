Rendera
=======

![Screenshot](https://raw.githubusercontent.com/JoeDavisson/rendera/master/screenshots/screenshot.png)
*Backround removed with fill tool, uses paint color for glow effect.*

## Overview
Rendera is a painting program for Linux and Windows. It's a little unusual in that brushstrokes are "rendered" in after being created by the user. There are no layers, but transparency information may be edited with relative ease, making it a useful companion to programs like Gimp or Inkscape.

## Windows Release
Package includes binaries for 32 and 64-bit Windows:

[rendera-0.2.5-b1.zip](https://github.com/JoeDavisson/rendera/releases/download/v0.2.5/rendera-0.2.5-b1.zip)

## Features
 * variety of painting and blending options
 * clone mode displays second cursor for visual assistance
 * colorize mode preserves luminosity for realistic results
 * high-quality color reduction and dithering
 * cut/paste selections
 * crop function
 * text tool works with blending modes and cloning
 * fill with edge feathering option
 * palette editor
 * interactive offset
 * "smart" grid snapping
 * bilinear/bicubic scaling
 * arbitrary rotation with scaling option
 * scaling, gaussian blur, and dithering are "gamma-aware"
 * multiple image support
 * multiple undo/redo for each image
 * file support: PNG, JPEG, BMP, TGA, GPL (Gimp Palette)
 * drag and drop image loading
 * image filters

## Build it from source
```$ git clone https://github.com/JoeDavisson/rendera.git```

```$ cd rendera```

Get FLTK 1.3.7:

```$ git submodule init```

```$ git submodule update```

Or uncompress the FLTK-1.3.7 source package here under `fltk`.

The Makefile supports ```linux``` and ```mingw``` cross-compiler targets (edit to choose).

Build fltk libraries (ignore errors about ```fluid.exe``` when using the mingw cross compiler):
```$ make fltklibs```

Run to create images header:
```$ make header```

Run whenever a header file is changed:
```$ make clean```

Then:
```$ make```

*Note: Compilation can be sped up by including ```-j <threads>``` after ```make```.*

## Dependencies

### Libraries
 * FLTK-1.3.7
 * libxft-dev (required for font rendering)
 * PNG and JPEG libraries are included in the FLTK source package.

Everything is statically linked resulting in a standalone executable.

### Toolchain
Rendera is built with ```gcc-11.2```.

