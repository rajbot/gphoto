#!/bin/bash

. "$(dirname $0)/utils/common.sh"

rm -rf "${cvsorig}" "${cvssrc}" "${distdir}" "${srcdir}" "${distroot}" "${instroot}"

rm -f install-sh missing mkinstalldirs aclocal.m4 configure config.log config.status
rm -f COPYING INSTALL
rm -f $(find . \( -name Makefile.in -or -name Makefile \) -print)
rm -rf autom4te.cache/

if [ "$1" = "--logout" ]
then
    while read CVSROOT
    do 
	if grep -q "${CVSROOT}" $HOME/.cvspass
	then
	    echo "##### Logging out from from ${CVSROOT}"
	    cvs -d "${CVSROOT}" logout
	else
	    echo "Good: already logged out from ${CVSROOT}."
	fi
    done < "${cvsmodulelist}"
fi
