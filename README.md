Rendera
=======

![Screenshot](https://raw.githubusercontent.com/Mortis69/rendera/master/screenshots/screenshot.png)

Rendera is a painting program suitable for photo-retouching and making seamless
textures. Originally for Windows, it has been rewritten from scratch using
C++/FLTK. Please see the Wiki for a list of features, and the credits file for a
list of contributors and contact info.

## Build it from source
```$ git clone https://github.com/Mortis69/rendera.git```

```$ cd rendera```

Uncompress the FLTK-1.3.3 source package here.

The Makefile supports ```linux``` and ```mingw``` cross-compiler targets.
(Edit the Makefile to choose.)

```$ make fltk```

```$ make```

## Dependencies

### Libraries

 * FLTK-1.3.3

### Toolchain

Rendera is built with ```gcc-4.6```. While ```clang-3.4``` usually works, it's
not "officially" supported.

