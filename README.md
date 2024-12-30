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

## Toolchain
Rendera is built with ```gcc-13.3```.

