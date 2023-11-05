AC_INIT([bluray_info], [1.12], [https://github.com/beandog/bluray_info/issues], [], [https://github.com/beandog/bluray_info])

dnl This is not a GNU package, so ignore required files / format
AM_INIT_AUTOMAKE([foreign])

dnl Check for C99 support
AC_PROG_CC_C99

dnl need math.h to do MBs calculations
AC_CHECK_HEADERS([math.h])

dnl Use pkg-config to check for libbluray
PKG_CHECK_MODULES([LIBBLURAY], [libbluray >= 1.2.0])

AC_ARG_WITH([libmpv], [AS_HELP_STRING([--with-libmpv], [Enable mpv support to build bluray_player])], [with_libmpv=${withval}], [with_libmpv=no])

dnl debugging
dnl AC_MSG_RESULT(x$with_libmpv)
dnl AC_MSG_RESULT(x$withval)

AS_IF([test "x$with_libmpv" != "xno"],
	[
		AC_DEFINE(HAVE_LIBMPV, [], [libmpv])
		PKG_CHECK_MODULES([MPV], [mpv])
	]
)

AM_CONDITIONAL([BLURAY_PLAYER], [test "x$with_libmpv" != "xno"])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
