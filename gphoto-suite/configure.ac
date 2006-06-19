AC_PREREQ(2.59)
dnl Use a short version and package name here, or "make dist" will fail
dnl packaging [...]/libgphoto2_port/gphoto2-port-portability-os2.h
dnl because tar only handles 99 chars per file path. Oh, and GNU tar 1.13
dnl will fail silently.
AC_INIT([gphoto suite],[20060619],[gphoto-devel@lists.sourceforge.net],[gphoto-suite-trunk])
AC_CONFIG_SRCDIR([checkout-config.sh])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([gnu 1.8 dist-bzip2])

GP_CHECK_SHELL_ENVIRONMENT
GP_CONFIG_MSG([Build])
GP_CONFIG_MSG([Source code location],[${srcdir}])


dnl ---------------------------------------------------------------------------
dnl compiler stuff
dnl ---------------------------------------------------------------------------
AC_PROG_CC
AC_C_CONST
_AM_PROG_TAR([pax])dnl 'pax' aka 'posix'
AM_PROG_LIBTOOL
AM_CFLAGS="$CFLAGS"
AM_LDFLAGS="$LDFLAGS"
AM_CPPFLAGS="$CPPFLAGS"
GP_CONFIG_MSG([Compiler],[${CC}])


GP_CONFIG_MSG([Features])


# ---------------------------------------------------------------------------
# i18n support
# ---------------------------------------------------------------------------
GP_GETTEXT_HACK([${PACKAGE}],[Hans Ulrich Niedermann])
ALL_LINGUAS="de"
AM_GNU_GETTEXT_VERSION([0.14.1])
AM_GNU_GETTEXT([external])
AM_PO_SUBDIRS()
AM_ICONV()
GP_GETTEXT_FLAGS()

# We cannot use [AC_DEFINE_UNQUOTED()] for these definitions, as
# we require make to do insert the proper $(datadir) value
AC_SUBST([localedir],["\$(datadir)/locale"])
AM_CPPFLAGS="$AM_CPPFLAGS -DLOCALEDIR=\\\"${localedir}\\\""


# ---------------------------------------------------------------------------
# Warnings: If we have GCC, be paranoid.
# ---------------------------------------------------------------------------
if test "x$GCC" = "xyes"; then
    AM_CPPFLAGS="$AM_CPPFLAGS -Wall -Wchar-subscripts -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wpointer-arith"
    AM_LDFLAGS="$AM_LDFLAGS -g -Wall"
fi

AC_SUBST([AM_CPPFLAGS])
AC_SUBST([AM_LDFLAGS])


dnl ---------------------------------------------------------------------------
dnl PKG_CONFIG and PKG_CONFIG_PATH setup
dnl ---------------------------------------------------------------------------
pwd="$(pwd)"
for dir in libexif/libexif libgphoto2/libgphoto2 libusb
do
	tmp="${pwd}/src/${dir}"
	if test "x${PKG_CONFIG_PATH}" = "x"; then
	   PKG_CONFIG_PATH="${tmp}"
	else
	   PKG_CONFIG_PATH="${tmp}:${PKG_CONFIG_PATH}"
	fi
done
export PKG_CONFIG_PATH
GP_PKG_CONFIG


# ---------------------------------------------------------------------------
# We check for GTK here so we can not configure the subdirs requiring
# GTK if it is missing.
# ---------------------------------------------------------------------------
PKG_CHECK_MODULES([GTK],[gtk+-2.0],[have_gtk=:],[have_gtk=false])
GP_CHECK_POPT([default-enabled])
if test "x$have_popt" = "xyes"; then
	have_popt=:
else
	have_popt=false
fi

# FIXME: Actually run test compilations to make sure that
#        we're catching the cross-compile case when the header/libs are
#        available for the build system, but not for the host system.

case "$host" in
*-linux*)
	have_libusb_system=:
	;;
*-freebsd*|*-openbsd*|*-netbsd*)
	have_libusb_system=:
	;;
*-darwin*)
	have_libusb_system=:
	;;
*-mingw32*)
	have_libusb_system=false
	have_gtk=false
	;;
*)
	have_libusb_system=false
	AC_MSG_WARN([
Warning:

This system does not seem to be supported by libusb.
There will be *NO* support for USB cameras on this system!

])
	;;
esac


# ---------------------------------------------------------------------------
# Configure subtrees
# ---------------------------------------------------------------------------
dnl Yes, the sequence matters!

# Initialize have_subdir_XXX variables
AC_DEFUN([GP_SUBMODULE],[dnl
AC_ARG_ENABLE([--disable-submodule-][$1],
	[AS_HELP_STRING([--disable-submodule-][$1],[disable ][$1][ submodule])],
[
	eval "enable_submod_$(echo "[$1]" | sed 'y/-/_/')=${enableval}"
],[
	eval "enable_submod_$(echo "[$1]" | sed 'y/-/_/')=yes"
])
eval "have_subdir_$(echo "[$1]" | sed 'y/-/_/')=no"
])dnl
GP_SUBMODULE([libexif])
GP_SUBMODULE([exif])
GP_SUBMODULE([libexif-gtk])
GP_SUBMODULE([gexif])
GP_SUBMODULE([libusb])
GP_SUBMODULE([libgphoto2])
GP_SUBMODULE([gphoto2])
GP_SUBMODULE([gphotofs])
GP_SUBMODULE([gtkam])


# Configure subdirs.
#   * take dependencies into account
#   * set special variables (XXX_LIBS and XXX_CPPFLAGS)
#   * FIXME: Add --disable-PACKAGE options to purposely disable certain ones.
   
   # This kind of relative definition only works for subprojects located 
   # in src/subproj, so we have to stick to that pattern.
   # Oh, and additionally, it works only with first level [AC_CONFIG_SUBDIRS]
   # calls, not on higher recursion levels. So there is a problem with
   # libgphoto2_port.
   # Relative
   gp_top_builddir="\$(top_builddir)/../.."
   gp_top_srcdir="\$(top_srcdir)/../.."
   # Absolute
   gp_top_builddir="$(pwd)"
   gp_top_srcdir="$(cd "${srcdir}" && pwd)"

if "$have_popt"; then
   :
else
   if test -d "${srcdir}/popt"; then
      AC_CONFIG_SUBDIRS([src/popt])
      AC_SUBST([POPT_CFLAGS],["-I${gp_top_srcdir}/src/popt"])
      AC_SUBST([POPT_LIBS],["${gp_top_builddir}/src/popt/libpopt.la"])
      have_popt=:
   else
      AC_MSG_WARN([
popt library neither found on the system nor is it shipped with
${PACKAGE_TARNAME}.
])
   fi
fi

if test -d ${srcdir}/src/libexif; then

   AC_CONFIG_SUBDIRS([src/libexif])
   GP_AUTOGEN_SUBDIR([src/libexif])

   AC_SUBST([LIBEXIF_CFLAGS],["-I${gp_top_builddir}/src/libexif -I${gp_top_srcdir}/src/libexif"])
   AC_SUBST([LIBEXIF_LIBS],["${gp_top_builddir}/src/libexif/libexif/libexif.la"])
   export LIBEXIF_LIBS LIBEXIF_CFLAGS

   if test ! -f ${srcdir}/src/libexif/configure; then
      AC_MSG_ERROR([
FATAL: ${PACKAGE_NAME} requires libexif source in src/libexif!
])
   fi
fi

   if "$have_popt" && test -d ${srcdir}/src/exif; then
      AC_CONFIG_SUBDIRS([src/exif])
      GP_AUTOGEN_SUBDIR([src/exif])
   fi

   if "$have_gtk" && test -d ${srcdir}/src/libexif-gtk; then
      AC_CONFIG_SUBDIRS([src/libexif-gtk])
      GP_AUTOGEN_SUBDIR([src/libexif-gtk])
      AC_SUBST([LIBEXIF_GTK_CFLAGS],["-I${gp_top_builddir}/src/libexif-gtk -I${gp_top_srcdir}/src/libexif-gtk"])
      AC_SUBST([LIBEXIF_GTK_LIBS],["${gp_top_builddir}/src/libexif-gtk/libexif-gtk/libexif-gtk.la"])
      export LIBEXIF_GTK_LIBS LIBEXIF_GTK_CFLAGS
      if test -d ${srcdir}/src/gexif; then
         AC_CONFIG_SUBDIRS([src/gexif])
         GP_AUTOGEN_SUBDIR([src/gexif])
      fi
   fi

   if "$have_libusb_system" && test -d ${srcdir}/src/libusb; then
      AC_CONFIG_SUBDIRS([src/libusb])
      GP_AUTOGEN_SUBDIR([src/libusb])
      # LIBUSB_LIBS="\$(top_builddir)/../../src/libusb/libusb.la"
      # Use absolute directory as workaround. LIBUSB_LIBS will be used
      # in libgphoto2_port, which is second-level [AC_CONFIG_SUBDIRS]
      # recursion.
      AC_SUBST([LIBUSB_CFLAGS],["-I${gp_top_builddir}/src/libusb -I${gp_top_srcdir}/src/libusb"])
      AC_SUBST([LIBUSB_LIBS],["${gp_top_builddir}/src/libusb/libusb.la"])
      export LIBUSB_LIBS LIBUSB_CFLAGS
   fi

   if test -d "$srcdir/src/gphoto2-manual"; then
      AC_CONFIG_SUBDIRS([src/gphoto2-manual])
      GP_AUTOGEN_SUBDIR([src/gphoto2-manual])
   fi

   if test -d ${srcdir}/src/libgphoto2; then
      AC_CONFIG_SUBDIRS([src/libgphoto2])
      GP_AUTOGEN_SUBDIR([src/libgphoto2])
      AC_SUBST([LIBGPHOTO2_CFLAGS],["-I${gp_top_builddir}/src/libgphoto2 -I${gp_top_builddir}/src/libgphoto2/gphoto2"])
      AC_SUBST([LIBGPHOTO2_LIBS],["${gp_top_builddir}/src/libgphoto2/libgphoto2/libgphoto2.la ${gp_top_builddir}/src/libgphoto2/libgphoto2_port/libgphoto2_port/libgphoto2_port.la"])
      export LIBGPHOTO2_LIBS LIBGPHOTO2_CFLAGS
      if test -d ${srcdir}/src/gphoto2; then
         AC_CONFIG_SUBDIRS([src/gphoto2])
         GP_AUTOGEN_SUBDIR([src/gphoto2])
      fi
      if test "x$have_submod_gphotofs" = "xyes" && test -d ${srcdir}/src/gphotofs; then
         AC_CONFIG_SUBDIRS([src/gphotofs])
         GP_AUTOGEN_SUBDIR([src/gphotofs])
      fi
      if "$have_gtk" && test -d ${srcdir}/src/libexif-gtk && test -d ${srcdir}/src/gtkam
      then
         AC_CONFIG_SUBDIRS([src/gtkam])      
         GP_AUTOGEN_SUBDIR([src/gtkam])
      fi
   fi

dnl else
dnl   AC_MSG_ERROR([
dnl FATAL: src/libexif subproject subdirectory not found.
dnl        The README file describes how to create subproject subdirs.
dnl ])
dnl fi

# set variables depending on subdirs
for sd in ${subdirs}; do
	varname="have_subdir_$(basename "$sd" | sed 'y/-/_/')"
	eval "${varname}=yes"
done


dnl ---------------------------------------------------------------------------
dnl Output files
dnl ---------------------------------------------------------------------------
AC_CONFIG_FILES([
  po/Makefile.in
  Makefile
  m4m/Makefile
  i18n-test/Makefile
])
AC_OUTPUT

GP_CONFIG_OUTPUT
