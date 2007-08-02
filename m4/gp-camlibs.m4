dnl GP_CAMLIB & Co.
dnl
dnl Redundancy free definition of libgphoto2 camlibs.
dnl
dnl Removes the redundany from the camlib lists, and executes
dnl additional consistency checks, e.g. to ensure that subdirectories
dnl actually exist.
dnl
dnl You can mark camlibs as obsolete, i.e. they won't be listed
dnl explicitly but still be recognized.
dnl
dnl Example usage:
dnl
dnl   GP_CAMLIB([canon])
dnl   GP_CAMLIB([casio])
dnl   [...]
dnl   GP_CAMLIB([ptp],[obsolete])
dnl   GP_CAMLIB([ptp2])
dnl   [...]
dnl   GP_CAMLIB([toshiba])
dnl   GP_CAMLIBS_DEFINE([camlibs])
dnl
dnl The camlibs basedir parameter of GP_CAMLIBS_DEFINE is optional.
dnl
dnl ####################################################################
dnl
AC_DEFUN([GP_CAMLIBS_INIT],[dnl
AC_BEFORE([$0],[GP_CAMLIB])dnl
m4_define_default([gp_camlib_srcdir], [camlibs])dnl
m4_define_default([gp_camlibs], [])dnl
m4_define_default([gp_camlibs_obsolete], [])dnl
])dnl
dnl
dnl ####################################################################
dnl
AC_DEFUN([GP_CAMLIB],[dnl
AC_REQUIRE([GP_CAMLIBS_INIT])dnl
AC_BEFORE([$0],[GP_CAMLIBS_DEFINE])dnl
m4_if([$2],[obsolete],[dnl
# $0($1,$2)
m4_append([gp_camlibs_obsolete], [$1], [ ])dnl
],[$#],[1],[dnl
# $0($1)
m4_append([gp_camlibs], [$1], [ ])dnl
],[dnl
m4_errprint(__file__:__line__:[ Error:
*** Illegal parameter 2 to $0: `$2'
*** Valid values are: undefined or [obsolete]
])dnl
m4_exit(1)dnl
])dnl
])dnl
dnl
dnl ####################################################################
dnl
AC_DEFUN([GP_CAMLIBS_DEFINE],[dnl
AC_REQUIRE([GP_CAMLIBS_INIT])dnl
m4_pattern_allow([m4_strip])dnl
m4_ifval([$1],[m4_define([gp_camlib_srcdir],[$1])])dnl
dnl for camlib in m4_strip(gp_camlibs) m4_strip(gp_camlibs_obsolete)
dnl do
dnl 	if test -d "$srcdir/m4_strip(gp_camlib_srcdir)/$camlib"; then :; else
dnl 		AC_MSG_ERROR([
dnl * Fatal:
dnl * Source subdirectory for camlib \`$camlib' not found in
dnl * directory \`$srcdir/m4_strip(gp_camlib_srcdir)/'
dnl ])
dnl 	fi
dnl done
AC_MSG_CHECKING([which camlibs to compile])
dnl Yes, that help output won't be all that pretty, but we at least
dnl do not have to edit it by hand.
AC_ARG_WITH([camlibs],[AS_HELP_STRING(
	[--with-camlibs=<list>],
	[Compile camera drivers (camlibs) in <list>. ]dnl
	[Drivers may be separated with commas. ]dnl
	[CAUTION: DRIVER NAMES AND CAMERA NAMES MAY DIFFER. ]dnl
	['all' is the default and compiles all camlibs. ]dnl
	[Possible camlibs are: ]dnl
	m4_strip(gp_camlibs))],
	[camlibs="$withval"],
	[camlibs="all"])dnl
dnl
ALL_DEFINED_CAMLIBS="m4_strip(gp_camlibs) m4_strip(gp_camlibs_obsolete)"
ALL_CURRENT_CAMLIBS="m4_strip(gp_camlibs)"
BUILD_THESE_CAMLIBS_BASE=""
if test "$camlibs" = "all"; then
	BUILD_THESE_CAMLIBS_BASE="$ALL_CURRENT_CAMLIBS"
	AC_MSG_RESULT([all])
else
	# camlibs=$(echo $camlibs | sed 's/,/ /g')
	IFS_save="$IFS"
	IFS=",$IFS"
	for camlib in ${camlibs}; do
		IFS="$IFS_save"
		found=false
		for from_all_camlib in ${ALL_DEFINED_CAMLIBS}; do
			if test "$camlib" = "$from_all_camlib"; then
				if test "x$BUILD_THESE_CAMLIBS_BASE" = "x"; then
					BUILD_THESE_CAMLIBS_BASE="$camlib"
				else
					BUILD_THESE_CAMLIBS_BASE="$BUILD_THESE_CAMLIBS_BASE $camlib"
				fi
				found=:
				break
			fi
		done
		if $found; then :; else
			AC_MSG_ERROR([Unknown camlib $camlib!])		
		fi
	done
	IFS="$IFS_save"
	AC_MSG_RESULT([$camlibs])
	AC_MSG_WARN([
Caution: You have chosen not to build all camlibs.
         Your camera may need one of those camlibs you disabled.
         YOUR CAMERA MAY NOT WORK!
])
	AC_DEFINE_UNQUOTED([INCOMPLETE_CAMLIB_SET], ["$BUILD_THESE_CAMLIBS_BASE"], 
                           [Whether the set of camlibs built is incomplete])
fi
BUILD_THESE_CAMLIBS=""
for f in $BUILD_THESE_CAMLIBS_BASE
do
    BUILD_THESE_CAMLIBS="${BUILD_THESE_CAMLIBS}${BUILD_THESE_CAMLIBS+ }${f}.la"
done
AC_SUBST([BUILD_THESE_CAMLIBS])
AC_SUBST([ALL_DEFINED_CAMLIBS])
AC_SUBST([ALL_CURRENT_CAMLIBS])
])dnl
dnl
dnl ####################################################################
dnl
dnl Local Variables:
dnl mode: autoconf
dnl End:
