Rendera
=======

![Screenshot](https://raw.githubusercontent.com/JoeDavisson/rendera/master/packaging/screenshot.png)

## Overview
Rendera is a painting program for Linux and Windows. It's a little unusual in that brushstrokes are "rendered" in after being created by the user. There are no layers, but transparency information may be edited with relative ease, making it a useful companion to programs like Gimp or Inkscape.

## News
Version 0.3.0 has a redesigned interface which allows scaling up and down with ctrl +/- keys. Text can now be rotated.

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

*Note: CMake support needs a bit of work, for now please use the Makefile which creates a self-contained executable.* 

### Dependencies
 * fltk-1.4.3 source
 * libxft-dev

See the Makefile for more information.

## Toolchain
Rendera is built with ```gcc-13.3```.

