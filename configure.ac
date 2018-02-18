AC_INIT([bluray_info], [1.2.0])

dnl This is not a GNU package, so ignore required files / format
AM_INIT_AUTOMAKE([foreign])

dnl Check for C99 support
AC_PROG_CC_C99

dnl Use pkg-config to check for libbluray
PKG_CHECK_MODULES([LIBBLURAY], [libbluray >= 1.0.0])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
