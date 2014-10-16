Rendera [![Build Status](https://travis-ci.org/Mortis69/rendera.svg?branch=master)](https://travis-ci.org/Mortis69/rendera)
=======

Rendera is a painting program suitable for photo-retouching and making seamless
textures. Originally for Windows, it has been rewritten from scratch using
C++/FLTK. Please see the Wiki for a list of features, and the credits file for a
list of contributors and contact info.

Should work on any platform supporting FLTK, (with a possible performance
penalty if Xlib is not available). Eventually we should have optimized versions
for Win/Mac, but we aren't quite there yet.

## Build it from source

```$ git clone https://github.com/Mortis69/rendera.git```

```$ cd rendera```

```$ autoreconf -vfi```

```$ ./configure```

```$ make```

You can also, optionally, run

```$ make check```

to run some unit tests. Note that Rendera supports the usual ```make``` targets,
including but not limited to

```$ make dist```

```$ make distcheck```

## Dependencies

### Libraries

 * FLTK 1.3
 * libjpeg
 * libpng
 * libz

### Toolchain

Rendera is built with ```gcc-4.6```. While ```clang-3.4``` usually works, it's
not "officially" supported. If you're building from source, you also need

 * ```autoconf-2.68```
 * ```automake-1.11```
 * ```libtool-2.4.2```

Rendera is developed on Ubuntu, so there are probably some additional, implicit
dependencies on the Linux system. Darwin and Cygwin might work, but your mileage
may vary.


![Screenshot](/screenshots/screenshot.png "Screenshot")

