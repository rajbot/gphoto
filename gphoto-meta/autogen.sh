#!/bin/sh
# Run this to generate all the initial makefiles, etc.
# This was lifted from the Gimp, and adapted slightly by
# Raph Levien.
# Since then, it has been rewritten quite a lot by misc. people.

# Call this file with AUTOCONF_SUFFIX and AUTOMAKE_SUFFIX set
# if you want us to call a specific version of autoconf or automake. 
# E.g. if you want us to call automake-1.6 instead of automake (which
# seems to be quite advisable if your automake is not already version 
# 1.6) then call this file with AUTOMAKE_SUFFIX set to "-1.6".

# Cases which are known to work:
# 2002-07-14: Debian GNU/Linux unstable with:
#   autoconf 2.53
#   automake 1.4-p5 and 1.6 (both)
#   gettext  0.10.40
#   libtool  1.4.2a
# 2002-07-14: Redhat Linux 7.3 with:
#   autoconf 2.53
#   automake 1.5 (not 1.4-p5)
#   gettext  0.11.1
#   libtool  1.4.2

DIE=0
srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.
test "$srcdir" = "." && srcdir=`pwd`

PROJECT=gphoto-meta

# failure subroutine.
# syntax: do-something || fail
fail() {
    status=$?
    echo "Last command failed with status $status in directory $(pwd)."
    echo "Aborting."
    exit $status
}

(autoconf${AUTOCONF_SUFFIX} --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "You must have autoconf installed to compile $PROJECT."
    echo "Download the appropriate package for your distribution,"
    echo "or get the source tarball at ftp://ftp.gnu.org/gnu/autoconf/"
    DIE=1
}

(automake${AUTOMAKE_SUFFIX} --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "You must have automake installed to compile $PROJECT."
    echo "Download the appropriate package for your distribution,"
    echo "or get the source tarball at ftp://ftp.gnu.org/gnu/automake/"
    DIE=1
}

(gettextize --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "You must have gettext installed to compile $PROJECT."
    echo "Download the appropriate package for your distribution,"
    echo "or get the source tarball at ftp://ftp.gnu.org/gnu/gettext/"
    DIE=1
}

(pkg-config --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "**Error**: You must have \`pkg-config installed."
	echo "Download the appropriate package for your distribution,"
	echo "or get the source tarball at http://www.freedesktop.org/software/pkgconfig/"
	DIE=1
}

if test "$DIE" -eq 1; then
    exit 1
fi

test -f bootstrap.sh || {
        echo "You must run this script in the top-level $PROJECT directory"
        exit 1
}

if ! ls dist-files/*.tar* >& /dev/null
then
    echo "You must run bootstrap.sh before autogen.sh"
    exit 1
fi

case "$CC" in
*xlc | *xlc\ * | *lcc | *lcc\ *) am_opt=--include-deps;;
esac


cd $srcdir || fail

    # We explicitly delete files and directories which are not contained 
    # in the CVS repository and which are generated by the build tools
    # called here.
    # This makes changing build tool versions much easier, and removes
    # the need for any --force parameters to the build tools which may
    # do something we do want or something we do not.

    echo "Cleaning stuff generated by aclocal"
    rm -f aclocal.m4

    echo "Running aclocal${AUTOMAKE_SUFFIX} $ACLOCAL_FLAGS"
    aclocal${AUTOMAKE_SUFFIX} $ACLOCAL_FLAGS || fail

    #echo "Cleaning stuff generated by autoheader"
    #rm -f config.h.in libgphoto2/config.h.in

    #echo "Running autoheader${AUTOCONF_SUFFIX}"
    #autoheader${AUTOCONF_SUFFIX} || fail

    echo "Cleaning stuff generated by automake"
    find . -name '*.am' -print | 
	while read file
	do # remove all .in files with a corresponding .am file
		rm -f $(echo "$file" | sed s/\.am\$/.in/g)
	done
    rm -f depcomp install-sh missing mkinstalldirs
    rm -f stamp-h* libgphoto2/stamp-h*

    echo "Running automake${AUTOMAKE_SUFFIX} --add-missing --gnu $am_opt"
    automake${AUTOMAKE_SUFFIX} --add-missing --gnu $am_opt || fail

    echo "Cleaning stuff generated by autoconf"
    rm -f configure
    rm -rf autom4te*.cache/

    echo "Running autoconf${AUTOCONF_SUFFIX}"
    autoconf${AUTOCONF_SUFFIX} || fail

echo
echo "$PROJECT is now ready for configuration."
