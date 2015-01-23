# rendera/configure.ac

AC_PREREQ([2.68])
AC_INIT([Rendera],
        [m4_esyscmd_s([git describe --always --dirty])],
        [joe_7@sbcglobal.net],
        [rendera],
        [https://github.com/Mortis69/rendera])

AC_CONFIG_AUX_DIR([.])

AC_LANG_PUSH([C++])
AC_CONFIG_SRCDIR([Rendera.md])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([subdir-objects])

LT_PREREQ([2.4])
LT_INIT([static disable-shared])


# Checks for programs.

AC_PROG_CXX
AC_PROG_LN_S
AC_PROG_MKDIR_P


# Checks for libraries.

AC_CHECK_LIB([X11],        [main],[], [AC_MSG_ERROR([libX11 missing])])
AC_CHECK_LIB([Xcursor],    [main],[], [AC_MSG_ERROR([libXcursor missing])])
AC_CHECK_LIB([Xext],       [main],[], [AC_MSG_ERROR([libXext missing])])
AC_CHECK_LIB([Xfixes],     [main],[], [AC_MSG_ERROR([libXfixes missing])])
AC_CHECK_LIB([Xft],        [main],[], [AC_MSG_ERROR([libXft missing])])
AC_CHECK_LIB([Xinerama],   [main],[], [AC_MSG_ERROR([libXinerama missing])])
AC_CHECK_LIB([dl],         [main],[], [AC_MSG_ERROR([libdl missing])])
AC_CHECK_LIB([fontconfig], [main],[], [AC_MSG_ERROR([libfontconfig missing])])
AC_CHECK_LIB([rt],         [main],[], [AC_MSG_ERROR([librt missing])])


# Checks for header files.

AC_CHECK_HEADER([cstdlib],[],[AC_MSG_ERROR([cstdlib missing])])
AC_CHECK_HEADER([stdint.h],[],[AC_MSG_ERROR([stdint.h missing])])
AC_CHECK_HEADER([unistd.h],[],[AC_MSG_ERROR([unistd.h missing])])
AC_CHECK_HEADER([limits.h],[],[AC_MSG_ERROR([limits.h missing])])


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

AC_CHECK_FUNCS([pow sqrt strcasecmp readlink])


# Checks for symbol declarations

AC_CHECK_DECLS([M_PI],
  [],
  [AC_MSG_ERROR([M_PI missing])],
  [[#include <math.h>]])

AC_CHECK_DECLS([PATH_MAX],
  [],
  [AC_MSG_ERROR([PATH_MAX missing])],
  [[#include <limits.h>]])


# various futzing

(
  $MKDIR_P share/rendera && \
  cd share/rendera       && \
  $LN_S ../../"${srcdir}"/data
)


AC_CONFIG_FILES([Makefile])

AC_OUTPUT
