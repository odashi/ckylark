#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ckylark], [0.2.0], [yus.takara@gmail.com])
AM_INIT_AUTOMAKE([ckylark], [0.2.0])
AC_CONFIG_SRCDIR([src/main.cc])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_LANG([C++])
AC_PROG_CXX
AC_PROG_CC

#AX_CXX_COMPILE_STDCXX_11
#AX_CHECK_COMPILE_FLAG([-std=c++11], [CXXFLAGS="$CXXFLAGS -std=c++11"])
CXXFLAGS="$CXXFLAGS -std=c++11"

CXXFLAGS="$CXXFLAGS -Wall"
CXXFLAGS="$CXXFLAGS -O2"
#CXXFLAGS="$CXXFLAGS -pg"

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
