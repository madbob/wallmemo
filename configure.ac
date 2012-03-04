m4_define([wallmemo_major_version], [0])
m4_define([wallmemo_minor_version], [1])
m4_define([wallmemo_micro_version], [0])

m4_define([wallmemo_version],
          [wallmemo_major_version.wallmemo_minor_version.wallmemo_micro_version])

m4_define([wallmemo_interface_age], [0])
m4_define([wallmemo_binary_age],
          [m4_eval(100 * wallmemo_minor_version + wallmemo_micro_version)])

m4_define([lt_current],
          [m4_eval(100 * wallmemo_minor_version + wallmemo_micro_version - wallmemo_interface_age)])
m4_define([lt_revision], [wallmemo_interface_age])
m4_define([lt_age],
          [m4_eval(wallmemo_binary_age - wallmemo_interface_age)])

m4_define([gio_req_version], [2.31.18])
m4_define([rsvg_req_version], [2.34.2])

AC_PREREQ([2.68])
AC_INIT([wallmemo], [wallmemo_version], [], [wallmemo])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.11])

GNOME_COMMON_INIT
GNOME_COMPILE_WARNINGS
GNOME_DEBUG_CHECK

IT_PROG_INTLTOOL([0.41.0])

AM_DISABLE_STATIC
AM_PATH_GLIB_2_0
AM_PROG_CC_C_O
AM_PROG_LIBTOOL

AC_HEADER_STDC
AC_C_CONST

WALLMEMO_VERSION=wallmemo_version
AC_SUBST(WALLMEMO_VERSION)

dnl wallmemo checks
PKG_CHECK_MODULES(WALLMEMO,
                  librsvg-2.0 >= rsvg_req_version dnl
                  gio-2.0 >= gio_req_version)
AC_SUBST(WALLMEMO_CFLAGS)
AC_SUBST(WALLMEMO_LIBS)

dnl = Enable debug level ===================================================

m4_define([debug_default],
          m4_if(m4_eval(wallmemo_minor_version % 2), [1], [yes], [minimum]))

AC_ARG_ENABLE(debug,
              AC_HELP_STRING([--enable-debug=@<:@no/minimum/yes@:>@],
                             [turn on debugging @<:@default=debug_default@:>@]),
,
              enable_debug=debug_default)

if test "x$enable_debug" = "xyes"; then
  test "$cflags_set" = set || CFLAGS="$CFLAGS -g"
  WALLMEMO_DEBUG_CFLAGS="-DWALLMEMO_ENABLE_DEBUG"
else
  if test "x$enable_debug" = "xno"; then
    WALLMEMO_DEBUG_CFLAGS="-DG_DISABLE_ASSERT -DG_DISABLE_CHECKS -DG_DISABLE_CAST_CHECKS"
  else # minimum
    WALLMEMO_DEBUG_CFLAGS="-DWALLMEMO_ENABLE_DEBUG -DG_DISABLE_CAST_CHECKS"
  fi
fi

AC_SUBST(WALLMEMO_DEBUG_CFLAGS)

dnl = Enable strict compiler flags =========================================

# use strict compiler flags only on development releases
#m4_define([maintainer_flags_default],
#          m4_if(m4_eval(wallmemo_minor_version % 2), [1], [yes], [no]))
m4_define([maintainer_flags_default], [no])
AC_ARG_ENABLE([maintainer-flags],
              AC_HELP_STRING([--enable-maintainer-flags=@<:@no/yes@:>@],
                             [Use strict compiler flags @<:@default=maintainer_flags_default@:>@]),,
              enable_maintainer_flags=maintainer_flags_default)

if test "x$enable_maintainer_flags" = "xyes"; then
  CPPFLAGS="$CPPFLAGS -g -Wall -Wshadow -Wcast-align -Wno-uninitialized -Werror"
else
  CPPFLAGS="$CPPFLAGS -g -Wall"
fi

dnl Internationalization
GETTEXT_PACKAGE=wallmemo
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], "$GETTEXT_PACKAGE", [Define the gettext package to use])
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_GNU_GETTEXT

AC_CONFIG_FILES([
        Makefile
        src/Makefile
        po/Makefile.in
])

AC_OUTPUT

echo ""
echo "               wallmemo: $VERSION"
echo ""
echo "                 Prefix: ${prefix}"
echo "         Compiler flags: ${CPPFLAGS}"
echo ""
