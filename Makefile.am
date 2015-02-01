# rendera/Makefile.am

SUBDIRS = tests

ACLOCAL_AMFLAGS = -I m4

# AM_CXXFLAGS = -Wall -Wno-unused -fmax-errors=5
AM_CPPFLAGS = -I $(srcdir)/src -isystem $(FLTK_CONFIG_INCLUDEDIR)
AM_CXXFLAGS = -Wall -Wextra -Werror -pedantic-errors # -rdynamic


noinst_LTLIBRARIES = libren.la

libren_la_CXXFLAGS = $(FLTK_CONFIG_CXXFLAGS) $(AM_CXXFLAGS)
libren_la_LDFLAGS  = $(FLTK_CONFIG_LDFLAGS)  $(AM_LDFLAGS)

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
  src/InputText.H         \
  src/InputText.cxx       \
  src/Map.H               \
  src/Map.cxx             \
  src/Math.H              \
  src/Octree.H            \
  src/Octree.cxx          \
  src/Offset.H            \
  src/Offset.cxx          \
  src/Paint.H             \
  src/Paint.cxx           \
  src/Palette.H           \
  src/Palette.cxx         \
  src/Path.H              \
  src/Path.cxx            \
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
  src/View.H              \
  src/View.cxx            \
  src/Widget.H            \
  src/Widget.cxx


bin_PROGRAMS = ./bin/rendera
__bin_rendera_SOURCES = src/Main.cxx
__bin_rendera_LDADD = libren.la


nobase_dist_pkgdata_DATA =          \
  data/icon48x48.png                \
  data/clone.png                    \
  data/constrain.png                \
  data/grid.png                     \
  data/logo.png                     \
  data/logo_large.png               \
  data/mirror.png                   \
  data/origin.png                   \
  data/shape.png                    \
  data/size.png                     \
  data/soft_edge.png                \
  data/stroke.png                   \
  data/tools.png                    \
  data/transparency.png             \
  data/wrap.png                     \
  data/zoom_fit.png                 \
  data/zoom_in.png                  \
  data/zoom_one.png                 \
  data/zoom_out.png


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

