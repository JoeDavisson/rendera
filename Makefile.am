# rendera/Makefile.am

SUBDIRS = tests

ACLOCAL_AMFLAGS = -I m4

#AM_CXXFLAGS = -Wall -Wno-unused -fmax-errors=5
AM_CXXFLAGS = -Wall -Wextra -Werror -pedantic-errors # -rdynamic


noinst_LTLIBRARIES = libren.la
libren_la_CXXFLAGS = $(FLTK_CONFIG_CXXFLAGS) $(AM_CXXFLAGS)
libren_la_LDFLAGS = $(FLTK_CONFIG_LDFLAGS)
libren_la_SOURCES =   \
  Bitmap.H            \
  Bitmap.cxx          \
  Blend.H             \
  Blend.cxx           \
  Brush.H             \
  Brush.cxx           \
  Button.H            \
  Button.cxx          \
  CheckBox.H          \
  CheckBox.cxx        \
  Clone.H             \
  Clone.cxx           \
  Common.H            \
  Crop.H              \
  Crop.cxx            \
  Dialog.H            \
  Dialog.cxx          \
  DialogWindow.H      \
  DialogWindow.cxx    \
  FastRnd.H           \
  Fill.H              \
  Fill.cxx            \
  FX.H                \
  FX.cxx              \
  File.H              \
  File.cxx            \
  FileSP.H            \
  FileSP.cxx          \
  Gamma.H             \
  GetColor.H          \
  GetColor.cxx        \
  Group.H             \
  Group.cxx           \
  Gui.H               \
  Gui.cxx             \
  Inline.H            \
  InputFloat.H        \
  InputFloat.cxx      \
  InputInt.H          \
  InputInt.cxx        \
  Map.H               \
  Map.cxx             \
  Octree.H            \
  Octree.cxx          \
  Offset.H            \
  Offset.cxx          \
  Paint.H             \
  Paint.cxx           \
  Palette.H           \
  Palette.cxx         \
  Project.H           \
  Project.cxx         \
  Quantize.H          \
  Quantize.cxx        \
  Render.H            \
  Render.cxx          \
  Rendera.H           \
  Separator.H         \
  Separator.cxx       \
  SP.H                \
  Stroke.H            \
  Stroke.cxx          \
  Text.H              \
  Text.cxx            \
  ToggleButton.H      \
  ToggleButton.cxx    \
  Tool.H              \
  Transform.H         \
  Transform.cxx       \
  Undo.H              \
  Undo.cxx            \
  View.H              \
  View.cxx            \
  Widget.H            \
  Widget.cxx          \
  getopt.h            \
  getopt.c


bin_PROGRAMS = ./bin/rendera
__bin_rendera_SOURCES = Main.cxx
__bin_rendera_LDADD = libren.la
# rendera_CXXFLAGS = $(FLTK_CONFIG_CXXFLAGS) $(AM_CXXFLAGS)
# rendera_LDFLAGS = $(FLTK_CONFIG_LDFLAGS)

# nodist_rendera_SOURCES = paths.h
# BUILT_SOURCES = paths.h
# CLEANFILES = paths.h
# # DISTCLEANFILES = paths.h
# paths.h : Makefile
# 	echo '#ifndef PATHS_H' >$@
# 	echo '#define PATHS_H' >>$@
# 	echo >>$@
# 	echo '#define DATADIR        "$(datadir)"'        >>$@
# 	echo '#define BINDIR         "$(bindir)"'         >>$@
# 	echo '#define DATADIR        "$(datadir)"'        >>$@
# 	echo '#define DATAROOTDIR    "$(datarootdir)"'    >>$@
# 	echo '#define DOCDIR         "$(docdir)"'         >>$@
# 	echo '#define DVIDIR         "$(dvidir)"'         >>$@
# 	echo '#define EXEC_PREFIX    "$(exec_prefix)"'    >>$@
# 	echo '#define HTMLDIR        "$(htmldir)"'        >>$@
# 	echo '#define INCLUDEDIR     "$(includedir)"'     >>$@
# 	echo '#define INFODIR        "$(infodir)"'        >>$@
# 	echo '#define LIBDIR         "$(libdir)"'         >>$@
# 	echo '#define LIBEXECDIR     "$(libexecdir)"'     >>$@
# 	echo '#define LOCALEDIR      "$(localedir)"'      >>$@
# 	echo '#define LOCALSTATEDIR  "$(localstatedir)"'  >>$@
# 	echo '#define MANDIR         "$(mandir)"'         >>$@
# 	echo '#define OLDINCLUDEDIR  "$(oldincludedir)"'  >>$@
# 	echo '#define PDFDIR         "$(pdfdir)"'         >>$@
# 	echo '#define PREFIX         "$(prefix)"'         >>$@
# 	echo '#define PSDIR          "$(psdir)"'          >>$@
# 	echo '#define SBINDIR        "$(sbindir)"'        >>$@
# 	echo '#define SHAREDSTATEDIR "$(sharedstatedir)"' >>$@
# 	echo '#define SYSCONFDIR     "$(sysconfdir)"'     >>$@
# 	echo '#define PKGDATADIR     "$(pkgdatadir)"'     >>$@
# 	echo >>$@
# 	echo '#endif//ndef PATHS_H' >>$@


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
libalgo_la_SOURCES = Algo.H Algo.cxx Types.H Types.cxx

test_blend_SOURCES = test/blend.C
test_bresenham_SOURCES = test/bresenham.C
test_bresenham_LDADD = libalgo.la
test_fltk_version_SOURCES = test/fltk_version.C
test_endian_SOURCES = test/endian.C
test_numerics_SOURCES = test/numerics.C


TESTS = $(check_PROGRAMS)


dist_man1_MANS = man/rendera.man

