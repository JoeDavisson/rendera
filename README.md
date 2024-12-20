Rendera
=======

![Screenshot](https://raw.githubusercontent.com/JoeDavisson/rendera/master/packaging/screenshot.png)

## Overview
Rendera is a painting program for Linux and Windows. It's a little unusual in that brushstrokes are "rendered" in after being created by the user. There are no layers, but transparency information may be edited with relative ease, making it a useful companion to programs like Gimp or Inkscape.

## Windows Release
Package includes binaries for 32 and 64-bit Windows:

[rendera-0.2.9.zip](https://github.com/JoeDavisson/rendera/releases/tag/v0.2.9)

## Features
 * variety of painting and blending options
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

## Get the Source
```$ git clone https://github.com/JoeDavisson/rendera.git```

## Build with CMake
```cmake .```  
```cmake --build .```

### Dependencies
 * libfltk13-dev
 * libpng-dev
 * libjpeg-dev

## Build with GNU Make
The GNU-Makefile may be used to create statically-linked executables for Linux and Windows.

```$ cd <path-to-rendera>```

Get FLTK 1.3.7:

```$ git submodule init```

```$ git submodule update```

Or uncompress the FLTK-1.3.7 source package here under `fltk`.

The Makefile supports ```linux``` and ```mingw``` cross-compiler targets (edit to choose).

Copy GNU-Makefile to Makefile (or use make -f). Build fltk libraries (ignore errors about ```fluid.exe``` when using the mingw cross compiler):
```$ make fltklib```

Run to create images header:
```$ make header```

Run whenever a header file is changed:
```$ make clean```

Then:
```$ make```

*Note: Compilation can be sped up by including ```-j <threads>``` after ```make```.*

### Dependencies
 * fltk source (see above)
 * libxft-dev (for font rendering)
 * libcanberra-gtk-module (if you want to get rid of the warning about it)

Static builds use the PNG and JPEG libraries included with FLTK. Everything is statically linked resulting in a standalone executable.

## Toolchain
Rendera is built with ```gcc-11.4```.

