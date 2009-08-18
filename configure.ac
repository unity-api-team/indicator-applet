
AC_INIT(src/applet-main.c)

AC_PREREQ(2.53)

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(indicator-applet, 0.2.0dev)

AM_MAINTAINER_MODE
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES])

IT_PROG_INTLTOOL([0.35.0])
AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_C_O
AC_STDC_HEADERS
AC_PROG_LIBTOOL

AC_SUBST(VERSION)
AC_CONFIG_MACRO_DIR([m4])

###########################
# Dependencies 
###########################

GTK_REQUIRED_VERSION=2.12
PANEL_REQUIRED_VERSION=2.0.0
DBUS_REQUIRED_VERSION=0.76
INDICATOR_REQUIRED_VERSION=0.1.0

PKG_CHECK_MODULES(APPLET, gtk+-2.0 >= $GTK_REQUIRED_VERSION
                          libpanelapplet-2.0 >= $PANEL_REQUIRED_VERSION
						  libgnomeui-2.0
						  indicator >= $INDICATOR_REQUIRED_VERSION)
AC_SUBST(APPLET_CFLAGS)
AC_SUBST(APPLET_LIBS)

AC_PATH_PROG(GCONFTOOL, gconftool-2)
AM_GCONF_SOURCE_2

##############################
# Custom Junk
##############################

AC_DEFUN([AC_DEFINE_PATH], [
  test "x$prefix" = xNONE && prefix="$ac_default_prefix"
  test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'
  ac_define_path=`eval echo [$]$2`
  ac_define_path=`eval echo [$]ac_define_path`
  $1="$ac_define_path"
  AC_SUBST($1)
  ifelse($3, ,
    AC_DEFINE_UNQUOTED($1, "$ac_define_path"),
    AC_DEFINE_UNQUOTED($1, "$ac_define_path", $3))
])

###########################
# Internationalization
###########################

GETTEXT_PACKAGE=indicator-applet
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Name of the default gettext domain])
AC_DEFINE_PATH(GNOMELOCALEDIR, "${datadir}/locale", [locale directory])

AM_GLIB_GNU_GETTEXT

##############################
# ./configure Flags messings
##############################

cflags_set=${CFLAGS+set}
# find the actual value for $prefix that we'll end up with
REAL_PREFIX=
if test "x$prefix" = "xNONE"; then
  REAL_PREFIX=$ac_default_prefix
else
  REAL_PREFIX=$prefix
fi

# Have to go $sysconfdir->$prefix/etc->/usr/local/etc   
# if you actually know how to code shell then fix this :-)
SYSCONFDIR_TMP="$sysconfdir"
old_prefix=$prefix
prefix=$REAL_PREFIX
EXPANDED_SYSCONFDIR=`eval echo $SYSCONFDIR_TMP`
prefix=$old_prefix
AC_SUBST(EXPANDED_SYSCONFDIR)

dnl Save flags to aclocal
ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

AC_DEFINE_PATH(PREFIX, "${prefix}", [prefix directory])
AC_DEFINE_PATH(SYSCONFDIR, "${sysconfdir}", [system configuration dir])
AC_DEFINE_PATH(LIBDIR, "${libdir}", [system configuration dir])

###########################
# Files
###########################

AC_OUTPUT([
Makefile
src/Makefile
src-session/Makefile
data/Makefile
po/Makefile.in
])

###########################
# Results
###########################

AC_MSG_NOTICE([

Indicator Applet Configuration:

	Prefix:                 $prefix
])
