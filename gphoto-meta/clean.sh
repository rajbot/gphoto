#!/bin/bash

. "$(dirname $0)/utils/common.sh"

cmd rm -rf "${cvsorig}" "${cvssrc}" "${distdir}" "${srcdir}" "${distroot}" "${instroot}"

cmd rm -f install-sh missing mkinstalldirs aclocal.m4 configure depcomp ltmain.sh
cmd rm -f config.log config.status config.guess config.sub
cmd rm -f COPYING INSTALL
cmd rm -f $(find . \( -name Makefile.in -or -name Makefile \) -print)
cmd rm -rf autom4te.cache/

if [ "$1" = "--logout" ]
then
    while read CVSROOT
    do 
	if grep -q "${CVSROOT}" $HOME/.cvspass
	then
	    echo "##### Logging out from from ${CVSROOT}"
	    cmd cvs -d "${CVSROOT}" logout
	else
	    echo "Good: already logged out from ${CVSROOT}."
	fi
    done < "${cvsmodulelist}"
fi
