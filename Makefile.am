# rendera/Makefile.am

ACLOCAL_AMFLAGS = -I m4

#AM_CXXFLAGS = -Wall -Wno-unused -fmax-errors=5
AM_CPPFLAGS = -I$(srcdir)/octree
AM_CXXFLAGS = -Wall -Wextra -Werror -pedantic-errors # -rdynamic


bin_PROGRAMS = rendera

rendera_SOURCES =     \
  Bitmap.cxx          \
  Bitmap.H            \
  Blend.cxx           \
  Blend.H             \
  Brush.cxx           \
  Brush.H             \
  Button.cxx          \
  Button.H            \
  Crop.cxx            \
  Crop.H              \
  Dialog.cxx          \
  Dialog.H            \
  FX.cxx              \
  FX.H                \
  Field.cxx           \
  Field.H             \
  File.cxx            \
  File.H              \
  GetColor.cxx        \
  GetColor.H          \
  Group.cxx           \
  Group.H             \
  Gui.cxx             \
  Gui.H               \
  Map.cxx             \
  Map.H               \
  Octree.cxx          \
  Octree.H            \
  Offset.cxx          \
  Offset.H            \
  Paint.cxx           \
  Paint.H             \
  Palette.cxx         \
  Palette.H           \
  Quantize.cxx        \
  Quantize.H          \
  Render.cxx          \
  Render.H            \
  Separator.cxx       \
  Separator.H         \
  Stroke.cxx          \
  Stroke.H            \
  ToggleButton.cxx    \
  ToggleButton.H      \
  Tool.cxx            \
  Tool.H              \
  Undo.cxx            \
  Undo.H              \
  View.cxx            \
  View.H              \
  Widget.cxx          \
  Widget.H            \
  Inline.H            \
  Main.cxx            \
  Rendera.H


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
  test_fltk_version \
  test_endian       \
  test_octree

test_fltk_version_SOURCES = test/fltk_version.C
test_endian_SOURCES = test/endian.C
test_octree_SOURCES = \
  octree/array2d.h    \
  octree/array.h      \
  octree/numtraits.h  \
  octree/octree.h     \
  octree/octree.tcc   \
  octree/point3d.h    \
  octree/shareddata.h \
  octree/tinyvector.h \
  test/octree.C


dist_doc_DATA =      \
  README.md          \
  octree/License.txt \
  octree/Readme.txt


TESTS = $(check_PROGRAMS)
