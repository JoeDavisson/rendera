# rendera/Makefile.am

ACLOCAL_AMFLAGS = -I m4

#AM_CXXFLAGS = -Wall -Wno-unused -fmax-errors=5
AM_CXXFLAGS = -Wall -Wextra # -Werror -pedantic-errors # -rdynamic


bin_PROGRAMS = rendera

rendera_SOURCES =     \
  Bitmap.cxx          \
  Bitmap.h            \
  Blend.cxx           \
  Blend.h             \
  Brush.cxx           \
  Brush.h             \
  Button.cxx          \
  Button.h            \
  Crop.cxx            \
  Crop.h              \
  Dialog.cxx          \
  Dialog.h            \
  FX.cxx              \
  FX.h                \
  Field.cxx           \
  Field.h             \
  File.cxx            \
  File.h              \
  GetColor.cxx        \
  GetColor.h          \
  Gui.cxx             \
  Gui.h               \
  Map.cxx             \
  Map.h               \
  Offset.cxx          \
  Offset.h            \
  Paint.cxx           \
  Paint.h             \
  Palette.cxx         \
  Palette.h           \
  Separator.cxx       \
  Separator.h         \
  Stroke.cxx          \
  Stroke.h            \
  ToggleButton.cxx    \
  ToggleButton.h      \
  Tool.cxx            \
  Tool.h              \
  View.cxx            \
  View.h              \
  Widget.cxx          \
  Widget.h            \
  common.h            \
  main.cxx            \
  Quantize.cxx        \
  Quantize.h          \
  rendera.h           \
  Undo.cxx            \
  Undo.h


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
  data/amount.png	\
  data/blend.png	\
  data/clone.png	\
  data/constrain.png    \
  data/display.png      \
  data/edge.png         \
  data/getvalue.png     \
  data/grid.png         \
  data/icon48x48.png    \
  data/invert.png	\
  data/lock.png         \
  data/logo.png         \
  data/logo_large.png   \
  data/minus.png	\
  data/mirror.png	\
  data/origin.png	\
  data/patterns.png     \
  data/plus.png         \
  data/shape.png	\
  data/size.png         \
  data/smooth.png	\
  data/soft_edge.png    \
  data/stroke.png	\
  data/tiny.png         \
  data/tools.png	\
  data/transparency.png \
  data/wrap.png         \
  data/zoom_fit.png     \
  data/zoom_in.png      \
  data/zoom_one.png     \
  data/zoom_out.png
