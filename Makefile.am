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
  Graphics/Bitmap.H            \
  Graphics/Bitmap.cxx          \
  Graphics/Blend.H             \
  Graphics/Blend.cxx           \
  Main/Brush.H             \
  Main/Brush.cxx           \
  Gui/Button.H            \
  Gui/Button.cxx          \
  Gui/CheckBox.H          \
  Gui/CheckBox.cxx        \
  Main/Clone.H             \
  Main/Clone.cxx           \
  Tools/Crop.H              \
  Tools/Crop.cxx            \
  Main/Dialog.H            \
  Main/Dialog.cxx          \
  Gui/DialogWindow.H      \
  Gui/DialogWindow.cxx    \
  Tools/Fill.H              \
  Tools/Fill.cxx            \
  FX/FilterMatrix.H      \
  FX/FX.H                \
  FX/FX.cxx              \
  File/File.H              \
  File/File.cxx            \
  File/FileSP.H            \
  File/FileSP.cxx          \
  Graphics/Gamma.H             \
  Tools/GetColor.H          \
  Tools/GetColor.cxx        \
  Gui/Group.H             \
  Gui/Group.cxx           \
  Main/Gui.H               \
  Main/Gui.cxx             \
  Images/Images.H            \
  Main/Inline.H            \
  Gui/InputFloat.H        \
  Gui/InputFloat.cxx      \
  Gui/InputInt.H          \
  Gui/InputInt.cxx        \
  Gui/InputText.H         \
  Gui/InputText.cxx       \
  Graphics/Map.H               \
  Graphics/Map.cxx             \
  Main/Math.cxx            \
  Main/Math.H              \
  Graphics/Octree.H            \
  Graphics/Octree.cxx          \
  Tools/Offset.H            \
  Tools/Offset.cxx          \
  Tools/Paint.H             \
  Tools/Paint.cxx           \
  Graphics/Palette.H           \
  Graphics/Palette.cxx         \
  Main/Project.H           \
  Main/Project.cxx         \
  Graphics/Quantize.H          \
  Graphics/Quantize.cxx        \
  Main/Render.H            \
  Main/Render.cxx          \
  Gui/Separator.H         \
  Gui/Separator.cxx       \
  Main/SP.H                \
  Main/Stroke.H            \
  Main/Stroke.cxx          \
  Tools/Text.H              \
  Tools/Text.cxx            \
  Gui/ToggleButton.H      \
  Gui/ToggleButton.cxx    \
  Tools/Tool.H              \
  FX/Transform.H         \
  FX/Transform.cxx       \
  Main/Undo.H              \
  Main/Undo.cxx            \
  Main/View.H              \
  Main/View.cxx            \
  Gui/Widget.H            \
  Gui/Widget.cxx


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

