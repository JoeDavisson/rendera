# rendera/Makefile.am

SUBDIRS = tests

ACLOCAL_AMFLAGS = -I m4

# AM_CXXFLAGS = -Wall -Wno-unused -fmax-errors=5
AM_CPPFLAGS = -I$(srcdir)/src
AM_CXXFLAGS = -Wall -Wextra -Werror -pedantic-errors # -rdynamic


noinst_LTLIBRARIES = libren.la
libren_la_CXXFLAGS = $(FLTK_CONFIG_CXXFLAGS) $(AM_CXXFLAGS)
libren_la_LDFLAGS = $(FLTK_CONFIG_LDFLAGS)
libren_la_SOURCES =       \
  src/Bitmap.H            \
  src/Bitmap.cxx          \
  src/Blend.H             \
  src/Blend.cxx           \
  src/Brush.H             \
  src/Brush.cxx           \
  src/Button.H            \
  src/Button.cxx          \
  src/CheckBox.H          \
  src/CheckBox.cxx        \
  src/Clone.H             \
  src/Clone.cxx           \
  src/Common.H            \
  src/Crop.H              \
  src/Crop.cxx            \
  src/Dialog.H            \
  src/Dialog.cxx          \
  src/DialogWindow.H      \
  src/DialogWindow.cxx    \
  src/FastRnd.H           \
  src/Fill.H              \
  src/Fill.cxx            \
  src/FX.H                \
  src/FX.cxx              \
  src/File.H              \
  src/File.cxx            \
  src/FileSP.H            \
  src/FileSP.cxx          \
  src/Gamma.H             \
  src/GetColor.H          \
  src/GetColor.cxx        \
  src/Group.H             \
  src/Group.cxx           \
  src/Gui.H               \
  src/Gui.cxx             \
  src/Inline.H            \
  src/InputFloat.H        \
  src/InputFloat.cxx      \
  src/InputInt.H          \
  src/InputInt.cxx        \
  src/Map.H               \
  src/Map.cxx             \
  src/Octree.H            \
  src/Octree.cxx          \
  src/Offset.H            \
  src/Offset.cxx          \
  src/Paint.H             \
  src/Paint.cxx           \
  src/Palette.H           \
  src/Palette.cxx         \
  src/Project.H           \
  src/Project.cxx         \
  src/Quantize.H          \
  src/Quantize.cxx        \
  src/Render.H            \
  src/Render.cxx          \
  src/Separator.H         \
  src/Separator.cxx       \
  src/SP.H                \
  src/Stroke.H            \
  src/Stroke.cxx          \
  src/Text.H              \
  src/Text.cxx            \
  src/ToggleButton.H      \
  src/ToggleButton.cxx    \
  src/Tool.H              \
  src/Transform.H         \
  src/Transform.cxx       \
  src/Undo.H              \
  src/Undo.cxx            \
  src/Util.H              \
  src/Util.cxx            \
  src/View.H              \
  src/View.cxx            \
  src/Widget.H            \
  src/Widget.cxx


bin_PROGRAMS = ./bin/rendera
__bin_rendera_SOURCES = src/Main.cxx
__bin_rendera_LDADD = libren.la
# rendera_CXXFLAGS = $(FLTK_CONFIG_CXXFLAGS) $(AM_CXXFLAGS)
# rendera_LDFLAGS = $(FLTK_CONFIG_LDFLAGS)


nobase_dist_pkgdata_DATA =          \
  data/common/icon48x48.png 

nobase_dist_pkgdata_DATA +=         \
  data/theme/dark/clone.png         \
  data/theme/dark/constrain.png     \
  data/theme/dark/grid.png          \
  data/theme/dark/logo.png          \
  data/theme/dark/logo_large.png    \
  data/theme/dark/mirror.png        \
  data/theme/dark/origin.png        \
  data/theme/dark/shape.png         \
  data/theme/dark/size.png          \
  data/theme/dark/soft_edge.png     \
  data/theme/dark/stroke.png        \
  data/theme/dark/tools.png         \
  data/theme/dark/transparency.png  \
  data/theme/dark/wrap.png          \
  data/theme/dark/zoom_fit.png      \
  data/theme/dark/zoom_in.png       \
  data/theme/dark/zoom_one.png      \
  data/theme/dark/zoom_out.png

nobase_dist_pkgdata_DATA +=         \
  data/theme/light/clone.png        \
  data/theme/light/constrain.png    \
  data/theme/light/grid.png         \
  data/theme/light/logo.png         \
  data/theme/light/logo_large.png   \
  data/theme/light/mirror.png       \
  data/theme/light/origin.png       \
  data/theme/light/shape.png        \
  data/theme/light/size.png         \
  data/theme/light/soft_edge.png    \
  data/theme/light/stroke.png       \
  data/theme/light/tools.png        \
  data/theme/light/transparency.png \
  data/theme/light/wrap.png         \
  data/theme/light/zoom_fit.png     \
  data/theme/light/zoom_in.png      \
  data/theme/light/zoom_one.png     \
  data/theme/light/zoom_out.png


check_PROGRAMS =    \
  test_blend        \
  test_bresenham    \
  test_fltk_version \
  test_endian       \
  test_numerics

check_LTLIBRARIES = libalgo.la
libalgo_la_SOURCES = \
  src/Algo.H         \
  src/Algo.cxx       \
  src/Types.H        \
  src/Types.cxx

test_blend_SOURCES = test/blend.C
test_bresenham_SOURCES = test/bresenham.C
test_bresenham_LDADD = libalgo.la
test_fltk_version_SOURCES = test/fltk_version.C
test_endian_SOURCES = test/endian.C
test_numerics_SOURCES = test/numerics.C


TESTS = $(check_PROGRAMS)


dist_man1_MANS = man/rendera.man

EXTRA_DIST = Rendera.md

