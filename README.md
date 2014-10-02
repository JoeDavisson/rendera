Rendera [![Build Status](https://travis-ci.org/Mortis69/rendera.svg?branch=master)](https://travis-ci.org/Mortis69/rendera)

Rendera is a painting program suitable for photo-retouching and making seamless textures. Originally for Windows, it has been rewritten from scratch using C++/FLTK. Please see the Wiki for a list of features, and the credits file for a list of contributors and contact info.

Should work on any platform supporting FLTK, (with a possible performance penalty if Xlib is not available). Eventually we should have optimized versions for Win/Mac, but we aren't quite there yet.

To compile, use "autoreconf -vfi" followed by "./configure" then "make", and optionally "make check" to run some unit tests. You will need to have FLTK 1.3, libjpeg, and libpng installed first.

![Screenshot](/screenshots/screenshot.png "Screenshot")

