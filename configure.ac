# rendera/configure.ac

AC_PREREQ([2.68])
AC_INIT([Rendera],
        [m4_esyscmd_s([git describe --always --dirty])],
        [joe_7@sbcglobal.net],
        [rendera],
        [https://github.com/Mortis69/rendera])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AC_EXEEXT

AC_LANG_PUSH([C++])
AC_CONFIG_SRCDIR([Rendera.md])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_TESTDIR([tests])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([subdir-objects])
LT_PREREQ([2.4])
LT_INIT([])


# spit platform info

AC_MSG_NOTICE([build_cpu     == $build_cpu])
AC_MSG_NOTICE([build_vendor  == $build_vendor])
AC_MSG_NOTICE([build_os      == $build_os])
AC_MSG_NOTICE([host_cpu      == $host_cpu])
AC_MSG_NOTICE([host_vendor   == $host_vendor])
AC_MSG_NOTICE([host_os       == $host_os])
AC_MSG_NOTICE([target_cpu    == $target_cpu])
AC_MSG_NOTICE([target_vendor == $target_vendor])
AC_MSG_NOTICE([target_os     == $target_os])

AC_DEFINE_UNQUOTED([BUILD_CPU],     [$build_cpu],     [define build cpu])
AC_DEFINE_UNQUOTED([BUILD_VENDOR],  [$build_vendor],  [define build vendor])
AC_DEFINE_UNQUOTED([BUILD_OS],      [$build_os],      [define build os])
AC_DEFINE_UNQUOTED([HOST_CPU],      [$host_cpu],      [define host cpu])
AC_DEFINE_UNQUOTED([HOST_VENDOR],   [$host_vendor],   [define host vendor])
AC_DEFINE_UNQUOTED([HOST_OS],       [$host_os],       [define host os])
AC_DEFINE_UNQUOTED([TARGET_CPU],    [$target_cpu],    [define target cpu])
AC_DEFINE_UNQUOTED([TARGET_VENDOR], [$target_vendor], [define target vendor])
AC_DEFINE_UNQUOTED([TARGET_OS],     [$target_os],     [define target os])


# spit executable extension

AC_MSG_NOTICE([EXEEXT == $EXEEXT])
AC_SUBST([EXEEXT])


# Checks for programs.

AC_PROG_CXX
AC_PROG_LN_S
AC_PROG_MKDIR_P
AC_CHECK_PROG([HAVE_FLTK_CONFIG],[fltk-config$EXEEXT],[yes],[no])


# Checks for libraries.

AC_CHECK_LIB([fltk], [main],[],[AC_MSG_ERROR([libfltk missing])])
AC_CHECK_LIB([jpeg], [main],[],[AC_MSG_ERROR([libjpeg missing])])
AC_CHECK_LIB([png],  [main],[],[AC_MSG_ERROR([libpng missing])])
AC_CHECK_LIB([z],    [main],[],[AC_MSG_ERROR([libz missing])])


# Checks for header files.

AC_CHECK_HEADER([cstdlib],[],[AC_MSG_ERROR([cstdlib missing])])
AC_CHECK_HEADER([stdint.h],[],[AC_MSG_ERROR([stdint.h missing])])
AC_CHECK_HEADER([unistd.h],[],[AC_MSG_ERROR([unistd.h missing])])
AC_CHECK_HEADER([limits.h],[],[AC_MSG_ERROR([limits.h missing])])
AC_CHECK_HEADER([getopt.h],[],[AC_MSG_ERROR([getopt.h missing])])


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


# platform-specific things

case $host_os in
  *darwin*)
    AC_MSG_NOTICE([doing darwin stuff])
    FLTK_CONFIG_INCLUDEDIR=$(fltk-config --includedir)
    FLTK_CONFIG_LDFLAGS=$(fltk-config --ldflags)
    ;;
  *linux*)
    AC_MSG_NOTICE([doing linux stuff])
    FLTK_CONFIG_INCLUDEDIR=$(fltk-config --includedir)
    FLTK_CONFIG_LDFLAGS=$(fltk-config --ldflags)
    AC_CHECK_LIB([X11],  [main],[],[AC_MSG_ERROR([libX11 missing])])
    ;;
  *)
    AC_MSG_ERROR([unsupported host os: $host_os])
    ;;
esac


# spit fltk-config stuff

AC_MSG_NOTICE([FLTK_CONFIG_INCLUDEDIR == $FLTK_CONFIG_INCLUDEDIR])
AC_MSG_NOTICE([FLTK_CONFIG_CXXFLAGS   == $FLTK_CONFIG_CXXFLAGS])
AC_MSG_NOTICE([FLTK_CONFIG_LDFLAGS    == $FLTK_CONFIG_LDFLAGS])
AC_SUBST([FLTK_CONFIG_INCLUDEDIR])
AC_SUBST([FLTK_CONFIG_CXXFLAGS])
AC_SUBST([FLTK_CONFIG_LDFLAGS])


AC_CONFIG_FILES([Makefile
                 tests/Makefile
                 tests/testsuite.at])

AC_OUTPUT

