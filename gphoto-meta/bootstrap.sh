#!/bin/bash

. "$(dirname $0)/utils/common.sh"
. "$(dirname $0)/utils/autodetect.sh"

########################################################################
# init - initialize stuff (CVS logins, etc)

init() {
    while read CVSROOT restofline
    do 
	CVSROOT="$(echo "${CVSROOT}" | sed 's|:/|:2401/|')"
	if grep -q "${CVSROOT}" $HOME/.cvspass
	then
	    echo "Good: already logged in for ${CVSROOT}."
	    continue
	fi
	echo "##### Logging in for ${CVSROOT}."
	echo "# Just press enter when asked for the CVS password:"
	cvs -d "${CVSROOT}" login
    done < "${cvsmodulelist}"
}

########################################################################
# download - download software from CVS into ${cvsorig}/MODULE

download() {
    echo "##### Getting software from CVS"
    cmd mkdir -p "${cvsorig}"
    cmd cd "${cvsorig}"
    while read CVSROOT module releasetag restofline
    do 
	if [ -d "${cvsorig}/${module}" ]
	then
	    echo "##### Not fetching module ${module} from CVS - it is already here."
	    continue
	fi
	echo "##### Checking out ${module} release ${releasetag} from ${CVSROOT}"
	if [ 'HEAD' = $releasetag ]
	then
	    releaseparm=""
	else
	    releaseparm="-r${releasetag}"
	fi
	cmd cvs -z3 -d "${CVSROOT}" ${releaseparm} checkout "${module}"
    done < "${cvsmodulelist}"
}

########################################################################
# preparedist - copy all stuff from ${cvsorig}/MODULE to ${cvssrc}/MODULE
#
# Thus, modifications to the source tree by autogen.sh doesn't mess up 
# our original CVS checkout.

preparedist() {
    [ -d "${cvssrc}" ] && return
    cmd rm -rf "${cvssrc}"
    cmd mkdir -p "${cvssrc}"
    # FIXME: relies on GNU cp
    cmd cp -a "${cvsorig}/"* "${cvssrc}/"
}

########################################################################
# builddist - build distribution tarball for all MODULEs

builddist() {
    subdirs=""
    export PKG_CONFIG_PATH="${distroot}/lib/pkgconfig:${PKG_CONFIG_PATH}"
    export LD_LIBRARY_PATH="${distroot}/lib:${LD_LIBRARY_PATH}"
    export PATH="${distroot}/bin:${PATH}"
    cmd mkdir -p "${distdir}"
    while read CVSROOT module releasetag autogenopts configopts restofline
    do
	if ls "${distdir}/${module}-"[0-9]* >& /dev/null
	then
	    echo "##### There is already a tarball for ${module} in ${distdir}"
	    echo "##### -> not rebuilding"
	else
	    echo "##### Creating fresh source tree..."
	    cmd rm -rf "${cvssrc}/${module}"
	    cmd cp -a "${cvsorig}/${module}" "${cvssrc}/"
	    echo "########################################################################"
	    echo "##### Creating distribution of ${module} now."
	    echo "########################################################################"
	    cmd cd "${cvssrc}/${module}"
	    echo "#### Press enter when asked to. And complain to the gettextize guys,"
	    echo "#    not to me."
	    if [ '""' = "$autogenopts" ]
	    then
		autogenopts=
	    fi
	    cmd ./autogen.sh --prefix="${distroot}" ${autogenopts}
	    if [ '""' = "$configopts" ]
	    then
		configopts=
	    fi
	    cmd ./configure --enable-maintainer-mode --prefix="${distroot}" ${configopts}
	    cmd make dist
	    cmd mv ${module}*.tar.gz "${distdir}/"
	    mv ${module}*.tar.bz2 "${distdir}/"
	    cmd make install
	fi
	subdirs="${subdirs} ${module}"
    done < "${cvsmodulelist}"
    (gphoto2 --version; gphoto2 --list-cameras) > "${distdir}/SUPPORTED-CAMERAS"
}

########################################################################
# create Makefile.am files

makefiles() {
    cmd cd "${distdir}"
    files=""
    while read CVSROOT module restofline
    do
	# add bz2 to dist if available, otherwise gz
	for tarball in "${distdir}/${module}-"[0-9]*.tar.{bz2,gz}
	do
	    if [ -s "${tarball}" ]
	    then
		files="${files} $(basename ${tarball})"
		dir="$(basename ${tarball} .tar.gz)"
		dir="$(basename ${dir} .tar.bz2)"
		tarball=""
		break
	    fi
	done
	[ "$tarball" != "" ] && echo "##### Warning: No tarball for ${module}"
    done < "${cvsmodulelist}"
    cat > "${distdir}/Makefile.am" <<EOF
# This file is autogenerated by $this. DO NOT MODIFY.

EXTRA_DIST = ${files} 
EOF
}


########################################################################
# main program

init

download

preparedist
builddist

makefiles
