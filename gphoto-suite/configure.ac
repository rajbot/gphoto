AC_PREREQ(2.59)
dnl Use a short version and package name here, or "make dist" will fail
dnl packaging [...]/libgphoto2_port/gphoto2-port-portability.h
dnl because tar only handles 99 chars per file path. Oh, and GNU tar 1.13
dnl even used to fail silently before automake got the filename-length-max=99
dnl parameter.
dnl
dnl If we want to use a package name like "gphoto-suite-2.2.4", then we have to
dnl remove libgphoto2_port from libgphoto2 and give it its own subdirectory:
dnl   src/libgphoto2_port/libgphoto2_port/gphoto2/gphoto2-port-portability.h
dnl   src/libgphoto2/libgphoto2/...
dnl   src/libgphoto2/camlibs/...
dnl   src/gphoto2/...
dnl That would change the standard module people check out to "gphoto-suite".
dnl
dnl "/src/libgphoto2/libgphoto2_port/libgphoto2_port/gphoto2/gphoto2-port-portability.h"
dnl is 82 characters, which leaves 17 characters for the distdir name. 
dnl "gphoto-suite-2.3.0" whould be 18 characters - one too many: 82+18=100 > 99 :-(
dnl
dnl However, "/src/libgphoto2/libgphoto2_port/gphoto2/gphoto2-port-portability.h"
dnl is 66 characters, which leaves 33 characters for the distdir name. 
dnl "gphoto-suite-2.3.0" whould be 18 characters - bingo!
AC_INIT([gphoto suite],[2.3.1.3.t],[gphoto-devel@lists.sourceforge.net],[gphoto-suite])
AC_CONFIG_SRCDIR([README.gphoto-suite])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([auto-aux])
AM_INIT_AUTOMAKE([-Wall -Werror gnu 1.9 dist-bzip2 check-news filename-length-max=99])


GP_CHECK_SHELL_ENVIRONMENT
GP_CONFIG_MSG([Build])
GP_CONFIG_MSG([Source code location],[${srcdir}])


dnl ---------------------------------------------------------------------------
dnl compiler stuff
dnl ---------------------------------------------------------------------------
AC_PROG_CC
AC_C_CONST
dnl Of course, Sun Solaris has not caught up with 10..20 year old POSIX standards.
dnl So for compatibility's sake, we restrict ourselves to classic tar with
dnl the 99 character limit and hope our path names are shorter.
dnl _AM_PROG_TAR([pax])dnl 'pax' aka 'posix'
AM_PROG_LIBTOOL
AM_CFLAGS="$CFLAGS"
AM_LDFLAGS="$LDFLAGS"
AM_CPPFLAGS="$CPPFLAGS"
GP_CONFIG_MSG([Compiler],[${CC}])


GP_CONFIG_MSG([Features])



# ---------------------------------------------------------------------------
# Warnings: If we have GCC, be paranoid.
# ---------------------------------------------------------------------------
if test "x$GCC" = "xyes"; then
    AM_CPPFLAGS="$AM_CPPFLAGS -Wall -Wextra -Wchar-subscripts -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wpointer-arith"
    AM_LDFLAGS="$AM_LDFLAGS -g -Wall"
fi

AC_SUBST([AM_CPPFLAGS])
AC_SUBST([AM_LDFLAGS])


# Configure subdirs.
   
dnl Absolute directories work with multiple levels of
dnl AC_CONFIG_SUBDIRS recursion, relative directories do not.
gp_top_builddir="$(pwd)"
gp_top_srcdir="$(cd "${srcdir}" && pwd)"

if test -d "$srcdir/src/gphoto2-manual"; then
   AC_CONFIG_SUBDIRS([src/gphoto2-manual])
fi

if test -d "${srcdir}/src/libgphoto2"; then
   AC_CONFIG_SUBDIRS([src/libgphoto2])
   # let pkg-config find *-uninstalled.pc
   PKG_CONFIG_PATH="${gp_top_builddir}/src/libgphoto2:${gp_top_builddir}/src/libgphoto2/libgphoto2_port${PKG_CONFIG_PATH+:}${PKG_CONFIG_PATH}"
   export PKG_CONFIG_PATH
fi

if test -d "${srcdir}/src/gphoto2"; then
   AC_CONFIG_SUBDIRS([src/gphoto2])
fi

if test -d "${srcdir}/src/gphotofs"; then
   AC_CONFIG_SUBDIRS([src/gphotofs])
fi

if test -d "${srcdir}/src/libgphoto2-sharp"; then
   AC_CONFIG_SUBDIRS([src/libgphoto2-sharp])
fi

if test -d "${srcdir}/src/libgphoto2-python"; then
   AC_CONFIG_SUBDIRS([src/libgphoto2-python])
fi


dnl ---------------------------------------------------------------------------
dnl Output files
dnl ---------------------------------------------------------------------------
AC_CONFIG_FILES([
  Makefile
  m4m/Makefile
])
AC_OUTPUT

GP_CONFIG_OUTPUT
