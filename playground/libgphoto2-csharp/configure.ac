AC_PREREQ(2.59)
AC_INIT([C# bindings for libgphoto2], [2.2.1.4.trunk], [gphoto-devel@lists.sourceforge.net], [libgphoto2-sharp])
AC_CONFIG_SRCDIR([src/CameraList.cs])
AC_CONFIG_MACRO_DIR([m4m])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([-Wall -Werror gnu 1.8 dist-bzip2])

GP_CHECK_LIBRARY([LIBGPHOTO2], [libgphoto2], [>= 2.2.1.4])

AC_CONFIG_FILES([
Makefile
src/Makefile
])
AC_OUTPUT
