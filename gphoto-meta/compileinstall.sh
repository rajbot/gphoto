#!/bin/bash

. "$(dirname $0)/utils/common.sh"
. "$(dirname $0)/utils/autodetect.sh"

if [ "$1" != "" ]
then
    mkdir -p "$1"
    if [ "$1" != "/" ] && [ -d "$1" ]
    then
	instroot="$1"
	echo "########################################################################"
	echo "# Installation root directory set to"
	echo "#     ${instroot}"
    else
	echo "$0: Cannot install into $1."
	exit 2
    fi
fi

########################################################################
# compile & install stuff

compileinstall() {
    if ! ls "${distdir}"/*.tar.gz >& /dev/null
    then
	echo "$0: Fatal: No source distributions found. Did you run bootstrap.sh?"
	exit 1
    fi
    cmd mkdir -p "${srcdir}"
    while read CVSROOT module restofline
    do
	for tarball in "${distdir}/${module}-"[0-9]*.tar.gz
	do
	    cmd cd "${srcdir}"
	    base="$(basename ${tarball} .tar.gz)"
	    if read dir < "${base}/installed-yet"
	    then
		if [ "$dir" = "${instroot}" ]
		then
		    echo "##### ${base} has already been installed to ${dir}."
		    echo "#     Not compiling/installing again."
		    break
		fi
	    fi
	    cmd rm -rf "${base}/"
	    cmd tar xvfz "${tarball}"
	    cmd cd "${srcdir}/${base}"
	    cmd ./configure --prefix="${instroot}"
	    cmd make install
	    echo -e "${instroot}\nRemove this file if you want gphoto-meta to rebuild this package" > installed-yet
	done
    done < "${cvsmodulelist}"

    echo
    echo "##########################################################################"
    echo "# Installed everything into ${instroot}"
    echo "# You may want to set the following variables to use the tools installed "
    echo "# there:"
    echo "##########################################################################"
    for var in PKG_CONFIG_PATH LD_LIBRARY_PATH PATH
    do
	echo "export $var=$(eval echo "\$$var")"
    done
    echo "########################################################################"
}

compileinstall
