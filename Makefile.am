# rendera/Makefile.am

SUBDIRS = tests

ACLOCAL_AMFLAGS = -I m4

INCLUDE=-I$(top_srcdir)/File -I$(top_srcdir)/FX -I$(top_srcdir)/Graphics -I$(top_srcdir)/Gui -I$(top_srcdir)/Images -I$(top_srcdir)/Main -I$(top_srcdir)/Tools

AM_CPPFLAGS = -isystem $(FLTK_CONFIG_INCLUDEDIR)
AM_CXXFLAGS = $(INCLUDE) -Wall -Wextra -Werror -pedantic-errors # -rdynamic


noinst_LTLIBRARIES = libren.la

libren_la_CXXFLAGS = $(INCLUDE) $(FLTK_CONFIG_CXXFLAGS) $(AM_CXXFLAGS)
libren_la_LDFLAGS  = $(FLTK_CONFIG_LDFLAGS)  $(AM_LDFLAGS)

libren_la_SOURCES =       \
  FX/FX.H                 \
  FX/FX.cxx               \
  FX/FilterMatrix.H       \
  FX/Transform.H          \
  FX/Transform.cxx        \
  File/File.H             \
  File/File.cxx           \
  File/FileSP.H           \
  File/FileSP.cxx         \
  Graphics/Bitmap.H       \
  Graphics/Bitmap.cxx     \
  Graphics/Blend.H        \
  Graphics/Blend.cxx      \
  Graphics/Gamma.H        \
  Graphics/Map.H          \
  Graphics/Map.cxx        \
  Graphics/Octree.H       \
  Graphics/Octree.cxx     \
  Graphics/Palette.H      \
  Graphics/Palette.cxx    \
  Graphics/Quantize.H     \
  Graphics/Quantize.cxx   \
  Gui/Button.H            \
  Gui/Button.cxx          \
  Gui/CheckBox.H          \
  Gui/CheckBox.cxx        \
  Gui/DialogWindow.H      \
  Gui/DialogWindow.cxx    \
  Gui/Group.H             \
  Gui/Group.cxx           \
  Gui/InputFloat.H        \
  Gui/InputFloat.cxx      \
  Gui/InputInt.H          \
  Gui/InputInt.cxx        \
  Gui/InputText.H         \
  Gui/InputText.cxx       \
  Gui/Separator.H         \
  Gui/Separator.cxx       \
  Gui/StaticText.H        \
  Gui/StaticText.cxx      \
  Gui/ToggleButton.H      \
  Gui/ToggleButton.cxx    \
  Gui/Widget.H            \
  Gui/Widget.cxx          \
  Images/Images.H         \
  Main/Brush.H            \
  Main/Brush.cxx          \
  Main/Clone.H            \
  Main/Clone.cxx          \
  Main/Dialog.H           \
  Main/Dialog.cxx         \
  Main/Gui.H              \
  Main/Gui.cxx            \
  Main/Inline.H           \
  Main/Math.H             \
  Main/Math.cxx           \
  Main/Project.H          \
  Main/Project.cxx        \
  Main/Render.H           \
  Main/Render.cxx         \
  Main/SP.H               \
  Main/Stroke.H           \
  Main/Stroke.cxx         \
  Main/Undo.H             \
  Main/Undo.cxx           \
  Main/View.H             \
  Main/View.cxx           \
  Tools/Crop.H            \
  Tools/Crop.cxx          \
  Tools/Fill.H            \
  Tools/Fill.cxx          \
  Tools/GetColor.H        \
  Tools/GetColor.cxx      \
  Tools/Offset.H          \
  Tools/Offset.cxx        \
  Tools/Paint.H           \
  Tools/Paint.cxx         \
  Tools/Text.H            \
  Tools/Text.cxx          \
  Tools/Tool.H


bin_PROGRAMS = ./rendera
__rendera_SOURCES = Main/Main.cxx
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

