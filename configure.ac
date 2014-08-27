# rendera/configure.ac

AC_PREREQ([2.68])
AC_INIT([Rendera],
        [0.1.0-snapshot],
        [joe_7@sbcglobal.net],
        [rendera],
        [https://github.com/Mortis69/rendera])
AC_LANG_PUSH([C++])
AC_CONFIG_SRCDIR([rendera.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([subdir-objects])
LT_PREREQ([2.4])
LT_INIT([dlopen])

# Checks for programs.
AC_PROG_CXX
# AX_CXX_COMPILE_STDCXX_11
# AC_PROG_CC


# Checks for libraries.

AC_CHECK_LIB([fltk],[fl_system],,
  AC_MSG_ERROR([libfltk missing]))

AC_CHECK_LIB([fltk_images],[fl_numericsort],,
  AC_MSG_ERROR([libfltk_images missing]))

AC_CHECK_LIB([jpeg],[jpeg_start_output],,
  AC_MSG_ERROR([libjpeg missing]))

AC_CHECK_LIB([png],[png_malloc],,
  AC_MSG_ERROR([libpng missing]))

AC_CHECK_LIB([z],[gzopen],,
  AC_MSG_ERROR([libz missing]))

AC_CHECK_LIB([X11],[XInitThreads],,
  AC_MSG_ERROR([libX11 missing]))


# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h])


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

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
