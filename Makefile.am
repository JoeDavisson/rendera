# rendera/Makefile.am

SUBDIRS = tests

ACLOCAL_AMFLAGS = -I m4

INCLUDE=-I$(top_srcdir)/src

AM_CPPFLAGS = -isystem $(FLTK_CONFIG_INCLUDEDIR)
AM_CXXFLAGS = $(INCLUDE) -Wall -Wextra -Werror -pedantic-errors # -rdynamic


noinst_LTLIBRARIES = libren.la

libren_la_CXXFLAGS = $(INCLUDE) $(FLTK_CONFIG_CXXFLAGS) $(AM_CXXFLAGS)
libren_la_LDFLAGS  = $(FLTK_CONFIG_LDFLAGS)  $(AM_LDFLAGS)

libren_la_SOURCES =       \
  src/FX.H                \
  src/FX.cxx              \
  src/FilterMatrix.H      \
  src/Transform.H         \
  src/Transform.cxx       \
  src/File.H              \
  src/File.cxx            \
  src/FileSP.H            \
  src/FileSP.cxx          \
  src/Bitmap.H            \
  src/Bitmap.cxx          \
  src/Blend.H             \
  src/Blend.cxx           \
  src/Gamma.H             \
  src/Map.H               \
  src/Map.cxx             \
  src/Octree.H            \
  src/Octree.cxx          \
  src/Palette.H           \
  src/Palette.cxx         \
  src/Quantize.H          \
  src/Quantize.cxx        \
  src/Button.H            \
  src/Button.cxx          \
  src/CheckBox.H          \
  src/CheckBox.cxx        \
  src/DialogWindow.H      \
  src/DialogWindow.cxx    \
  src/Group.H             \
  src/Group.cxx           \
  src/InputFloat.H        \
  src/InputFloat.cxx      \
  src/InputInt.H          \
  src/InputInt.cxx        \
  src/InputText.H         \
  src/InputText.cxx       \
  src/Separator.H         \
  src/Separator.cxx       \
  src/StaticText.H        \
  src/StaticText.cxx      \
  src/ToggleButton.H      \
  src/ToggleButton.cxx    \
  src/Widget.H            \
  src/Widget.cxx          \
  src/Images.H            \
  src/Brush.H             \
  src/Brush.cxx           \
  src/Clone.H             \
  src/Clone.cxx           \
  src/Dialog.H            \
  src/Dialog.cxx          \
  src/Gui.H               \
  src/Gui.cxx             \
  src/Inline.H            \
  src/Math.H              \
  src/Math.cxx            \
  src/Project.H           \
  src/Project.cxx         \
  src/Render.H            \
  src/Render.cxx          \
  src/SP.H                \
  src/Stroke.H            \
  src/Stroke.cxx          \
  src/Undo.H              \
  src/Undo.cxx            \
  src/View.H              \
  src/View.cxx            \
  src/Crop.H              \
  src/Crop.cxx            \
  src/Fill.H              \
  src/Fill.cxx            \
  src/GetColor.H          \
  src/GetColor.cxx        \
  src/Offset.H            \
  src/Offset.cxx          \
  src/Paint.H             \
  src/Paint.cxx           \
  src/Text.H              \
  src/Text.cxx            \
  src/Tool.H


bin_PROGRAMS = ./rendera
__rendera_SOURCES = src/Main.cxx
__rendera_LDADD = libren.la

check_PROGRAMS =    \
  test_blend        \
  test_bresenham    \
  test_fltk_version \
  test_endian       \
  test_numerics

check_LTLIBRARIES = libalgo.la
libalgo_la_SOURCES = \
  test/Algo.H         \
  test/Algo.cxx       \
  test/Types.H        \
  test/Types.cxx

test_blend_SOURCES = test/blend.C

test_bresenham_SOURCES = test/bresenham.C
test_bresenham_LDADD = libalgo.la

test_fltk_version_SOURCES = test/fltk_version.C

test_endian_SOURCES = test/endian.C

test_numerics_SOURCES = test/numerics.C


TESTS = $(check_PROGRAMS)


dist_man1_MANS = man/rendera.man

EXTRA_DIST = Images Rendera.md Images.md

