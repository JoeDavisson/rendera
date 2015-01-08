# rendera/configure.ac

AC_PREREQ([2.68])
AC_INIT([Rendera],
        [m4_esyscmd_s([git describe --always --dirty])],
        [joe_7@sbcglobal.net],
        [rendera],
        [https://github.com/Mortis69/rendera])
AC_LANG_PUSH([C++])
AC_CONFIG_SRCDIR([Rendera.H])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_TESTDIR([tests])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([subdir-objects])
LT_PREREQ([2.4])
LT_INIT([dlopen])

# Checks for programs.
AC_PROG_CXX
AC_PROG_LN_S
AC_PROG_MKDIR_P
# AX_CXX_COMPILE_STDCXX_11
# AC_PROG_CC


# Checks for libraries.

dnl AC_CHECK_LIB([dl],[dlsym],[],
dnl   [AC_MSG_ERROR([libdl missing])])

AC_CHECK_LIB([fltk],[fl_system],[],
  [AC_MSG_ERROR([libfltk missing])])

#AC_CHECK_LIB([fltk_images],[fl_numericsort],[],
#  [AC_MSG_ERROR([libfltk_images missing])])

AC_CHECK_LIB([jpeg],[jpeg_start_output],[],
  [AC_MSG_ERROR([libjpeg missing])])

AC_CHECK_LIB([png],[png_malloc],[],
  [AC_MSG_ERROR([libpng missing])])

AC_CHECK_LIB([rt],[clock_gettime],[],
  [AC_MSG_ERROR([librt missing])])

AC_CHECK_LIB([z],[gzopen],[],
  [AC_MSG_ERROR([libz missing])])

AC_CHECK_LIB([X11],[XInitThreads],[],
  [AC_MSG_ERROR([libX11 missing])])


dnl Checks for header files.

AC_CHECK_HEADER([cstdlib],[],[AC_MSG_ERROR([cstdlib missing])])
AC_CHECK_HEADER([endian.h],[],[AC_MSG_ERROR([endian.h missing])])
AC_CHECK_HEADER([stdint.h],[],[AC_MSG_ERROR([stdint.h missing])])


# Checks for typedefs, structures, and compiler characteristics.

AC_C_INLINE

AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T


# Checks for library functions.

AC_CHECK_FUNCS([pow sqrt strcasecmp])


dnl Checks for symbol declarations

AC_CHECK_DECLS([M_PI],
  [],
  [AC_MSG_ERROR([M_PI missing])],
  [[#include <math.h>]])


# various futzing

FLTK_CONFIG_CXXFLAGS=$(fltk-config --cxxflags)
FLTK_CONFIG_LDFLAGS=$(fltk-config --ldflags)
AC_SUBST([FLTK_CONFIG_CXXFLAGS])
AC_SUBST([FLTK_CONFIG_LDFLAGS])

(
  $MKDIR_P share/rendera && \
  cd share/rendera       && \
  $LN_S ../../"${srcdir}"/data
)


AC_CONFIG_FILES([Makefile
                 tests/Makefile
                 tests/testsuite.at])

AC_OUTPUT
