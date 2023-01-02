Rendera
=======

![Screenshot](https://raw.githubusercontent.com/JoeDavisson/rendera/master/screenshots/screenshot.png)

Rendera is a painting program suitable for photo-retouching and making seamless
textures. Originally a C program for Windows, it has been rewritten from scratch using
C++/FLTK.

## Build it from source
```$ git clone https://github.com/JoeDavisson/rendera.git```

```$ cd rendera```

Get FLTK 1.3.7:

```$ git submodule init```

```$ git submodule update```

Or uncompress the FLTK-1.3.7 source package here under `fltk`.

The Makefile supports ```linux``` and ```mingw``` cross-compiler targets.
(Edit the Makefile to choose.)

```$ make fltklibs```

Do this the first time anyway to build the images header:
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

