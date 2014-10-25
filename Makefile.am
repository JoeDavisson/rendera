# rendera/Makefile.am

ACLOCAL_AMFLAGS = -I m4

#AM_CXXFLAGS = -Wall -Wno-unused -fmax-errors=5
AM_CXXFLAGS = -Wall -Wextra -Werror -pedantic-errors # -rdynamic


bin_PROGRAMS = rendera

rendera_SOURCES =     \
  Bitmap.H            \
  Bitmap.cxx          \
  Blend.H             \
  Blend.cxx           \
  Brush.H             \
  Brush.cxx           \
  Button.H            \
  Button.cxx          \
  Common.H            \
  Crop.H              \
  Crop.cxx            \
  Dialog.H            \
  Dialog.cxx          \
  FX.H                \
  FX.cxx              \
  File.H              \
  File.cxx            \
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
  Main.cxx            \
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
  Stroke.H            \
  Stroke.cxx          \
  Text.H              \
  Text.cxx            \
  ToggleButton.H      \
  ToggleButton.cxx    \
  Tool.H              \
  Tool.cxx            \
  Transform.H         \
  Transform.cxx       \
  Undo.H              \
  Undo.cxx            \
  View.H              \
  View.cxx            \
  Widget.H            \
  Widget.cxx          


rendera_LDFLAGS = $(shell fltk-config --ldflags)
rendera_CXXFLAGS = $(shell fltk-config --cxxflags)


nodist_rendera_SOURCES = paths.h
BUILT_SOURCES = paths.h
CLEANFILES = paths.h
# DISTCLEANFILES = paths.h
paths.h : Makefile
	echo '#ifndef PATHS_H' >$@
	echo '#define PATHS_H' >>$@
	echo >>$@
	echo '#define DATADIR        "$(datadir)"'        >>$@
	echo '#define BINDIR         "$(bindir)"'         >>$@
	echo '#define DATADIR        "$(datadir)"'        >>$@
	echo '#define DATAROOTDIR    "$(datarootdir)"'    >>$@
	echo '#define DOCDIR         "$(docdir)"'         >>$@
	echo '#define DVIDIR         "$(dvidir)"'         >>$@
	echo '#define EXEC_PREFIX    "$(exec_prefix)"'    >>$@
	echo '#define HTMLDIR        "$(htmldir)"'        >>$@
	echo '#define INCLUDEDIR     "$(includedir)"'     >>$@
	echo '#define INFODIR        "$(infodir)"'        >>$@
	echo '#define LIBDIR         "$(libdir)"'         >>$@
	echo '#define LIBEXECDIR     "$(libexecdir)"'     >>$@
	echo '#define LOCALEDIR      "$(localedir)"'      >>$@
	echo '#define LOCALSTATEDIR  "$(localstatedir)"'  >>$@
	echo '#define MANDIR         "$(mandir)"'         >>$@
	echo '#define OLDINCLUDEDIR  "$(oldincludedir)"'  >>$@
	echo '#define PDFDIR         "$(pdfdir)"'         >>$@
	echo '#define PREFIX         "$(prefix)"'         >>$@
	echo '#define PSDIR          "$(psdir)"'          >>$@
	echo '#define SBINDIR        "$(sbindir)"'        >>$@
	echo '#define SHAREDSTATEDIR "$(sharedstatedir)"' >>$@
	echo '#define SYSCONFDIR     "$(sysconfdir)"'     >>$@
	echo '#define PKGDATADIR     "$(pkgdatadir)"'     >>$@
	echo >>$@
	echo '#endif//ndef PATHS_H' >>$@


dist_pkgdata_DATA =     \
  data/clone.png	\
  data/constrain.png    \
  data/grid.png         \
  data/icon48x48.png    \
  data/logo.png         \
  data/logo_large.png   \
  data/mirror.png	\
  data/origin.png	\
  data/shape.png	\
  data/size.png         \
  data/soft_edge.png    \
  data/stroke.png	\
  data/tools.png	\
  data/transparency.png \
  data/wrap.png         \
  data/zoom_fit.png     \
  data/zoom_in.png      \
  data/zoom_one.png     \
  data/zoom_out.png


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
