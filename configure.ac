AC_INIT([bluray_info], [1.5], [https://github.com/beandog/bluray_info/issues], [], [http://dvds.beandog.org/])

dnl This is not a GNU package, so ignore required files / format
AM_INIT_AUTOMAKE([foreign])

dnl Check for C99 support
AC_PROG_CC_C99

AC_CHECK_HEADERS([math.h])

dnl Use pkg-config to check for libbluray
PKG_CHECK_MODULES([LIBBLURAY], [libbluray >= 1.0.0])

dnl Using libmpv for the player is optional, but enabled by default
AC_ARG_WITH([libmpv], [AS_HELP_STRING([--with-libmpv], [Enable libmpv support for player])], [PKG_CHECK_MODULES([MPV], [mpv >= 1.25.0], [
	with_libmpv=yes
	AC_DEFINE(HAVE_LIBMPV, [], [libmpv])
])])

AM_CONDITIONAL([BLURAY_PLAYER], [test "x$with_libmpv" = xyes])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
