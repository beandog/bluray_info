AC_INIT([bluray_info], [1.11], [https://github.com/beandog/bluray_info/issues], [], [https://github.com/beandog/bluray_info])

dnl This is not a GNU package, so ignore required files / format
AM_INIT_AUTOMAKE([foreign])

dnl Check for C99 support
AC_PROG_CC_C99

dnl need math.h to do MBs calculations
AC_CHECK_HEADERS([math.h])

dnl Use pkg-config to check for libbluray
PKG_CHECK_MODULES([LIBBLURAY], [libbluray >= 1.2.0])

dnl Use pkg-config to check for libaacs
PKG_CHECK_MODULES([LIBAACS], [libaacs])

dnl Using libmpv for the player is optional, but enabled by default
AC_ARG_WITH([libmpv], [AS_HELP_STRING([--with-libmpv], [Enable libmpv support to build bluray_player])], [PKG_CHECK_MODULES([MPV], [mpv], [
	with_libmpv=yes
	AC_DEFINE(HAVE_LIBMPV, [], [libmpv])
])])

AM_CONDITIONAL([BLURAY_PLAYER], [test "x$with_libmpv" = xyes])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
