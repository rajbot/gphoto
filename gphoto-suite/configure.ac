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
AC_INIT([gphoto suite],[2.4.0.4],[gphoto-devel@lists.sourceforge.net],[gphoto-suite])
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
gp_top_builddir="$(pwd)/"
gp_top_srcdir="$(cd "${srcdir}" && pwd)/"

GP_SUBPACKAGE([gphoto2_manual])
if "$enable_subpackage_gphoto2_manual" && test -d "${srcdir}/src/gphoto2-manual"; then
   AC_CONFIG_SUBDIRS([src/gphoto2-manual])
   # let pkg-config find *-uninstalled.pc
   PKG_CONFIG_PATH="${gp_top_builddir}src/gphoto2-manual${PKG_CONFIG_PATH+:}${PKG_CONFIG_PATH}"
   export PKG_CONFIG_PATH
fi

GP_SUBPACKAGE([libexif])
if "$enable_subpackage_libexif" && test -d "${srcdir}/src/libexif"; then
   AC_CONFIG_SUBDIRS([src/libexif])
   # let pkg-config find *-uninstalled.pc
   PKG_CONFIG_PATH="${gp_top_builddir}src/libexif${PKG_CONFIG_PATH+:}${PKG_CONFIG_PATH}"
   export PKG_CONFIG_PATH
fi

GP_SUBPACKAGE([exif])
if "$enable_subpackage_exif" && test -d "${srcdir}/src/exif"; then
   AC_CONFIG_SUBDIRS([src/exif])
fi

GP_SUBPACKAGE([libexif_gtk])
if "$enable_subpackage_libexif_gtk" && test -d "${srcdir}/src/libexif-gtk"; then
   AC_CONFIG_SUBDIRS([src/libexif-gtk])
   # let pkg-config find *-uninstalled.pc
   PKG_CONFIG_PATH="${gp_top_builddir}src/libexif-gtk${PKG_CONFIG_PATH+:}${PKG_CONFIG_PATH}"
   export PKG_CONFIG_PATH
fi

GP_SUBPACKAGE([gexif])
if "$enable_subpackage_gexif" && test -d "${srcdir}/src/gexif"; then
   AC_CONFIG_SUBDIRS([src/gexif])
fi

GP_SUBPACKAGE([libgphoto2])
if "$enable_subpackage_libgphoto2" && test -d "${srcdir}/src/libgphoto2"; then
   AC_CONFIG_SUBDIRS([src/libgphoto2])
   # let pkg-config find *-uninstalled.pc
   PKG_CONFIG_PATH="${gp_top_builddir}src/libgphoto2/libgphoto2_port${PKG_CONFIG_PATH+:}${PKG_CONFIG_PATH}"
   PKG_CONFIG_PATH="${gp_top_builddir}src/libgphoto2${PKG_CONFIG_PATH+:}${PKG_CONFIG_PATH}"
   # PKG_CONFIG_PATH="src/libgphoto2/libgphoto2_port${PKG_CONFIG_PATH+:}${PKG_CONFIG_PATH}"
   # PKG_CONFIG_PATH="src/libgphoto2${PKG_CONFIG_PATH+:}${PKG_CONFIG_PATH}"
   export PKG_CONFIG_PATH
fi

GP_SUBPACKAGE([gphoto2])
if "$enable_subpackage_gphoto2" && test -d "${srcdir}/src/gphoto2"; then
   AC_CONFIG_SUBDIRS([src/gphoto2])
fi

GP_SUBPACKAGE([gphotofs])
if "$enable_subpackage_gphotofs" && test -d "${srcdir}/src/gphotofs"; then
   AC_CONFIG_SUBDIRS([src/gphotofs])
fi

GP_SUBPACKAGE([libgphoto2_sharp])
if "$enable_subpackage_libgphoto2_sharp" && test -d "${srcdir}/src/libgphoto2-sharp"; then
   AC_CONFIG_SUBDIRS([src/libgphoto2-sharp])
fi

GP_SUBPACKAGE([libgphoto2_python])
if "$enable_subpackage_libgphoto2_python" && test -d "${srcdir}/src/libgphoto2-python"; then
   AC_CONFIG_SUBDIRS([src/libgphoto2-python])
fi

GP_SUBPACKAGE([gtkam])
if "$enable_subpackage_gtkam" && test -d "${srcdir}/src/gtkam"; then
   AC_CONFIG_SUBDIRS([src/gtkam])
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

