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
    if ! ls "${distdir}"/*.tar.{gz,bz2} >& /dev/null
    then
	echo "$0: Fatal: No source distributions found. Did you run bootstrap.sh?"
	exit 1
    fi

    if [ -d "$toolroot" ]
    then
	echo "##### Going to use/build our own tools from/to ${toolroot}"
	echo "    # If you don't want to use these tools, remove the"
	echo "    # ${toolroot} directory."
	# these are global variables
	PATH="${toolroot}/bin:${PATH}"
	LD_LIBRARY_PATH="${toolroot}/lib:${LD_LIBRARY_PATH}"
	export PATH LD_LIBRARY_PATH
    fi

    cmd mkdir -p "${srcdir}"
    export PKG_CONFIG_PATH="${instroot}/lib/pkgconfig:${PKG_CONFIG_PATH}"
    export LD_LIBRARY_PATH="${instroot}/lib:${LD_LIBRARY_PATH}"
    export PATH="${instroot}/bin:${PATH}"
    while read CVSROOT module restofline
    do
	# unpack gz if available, else bz2 (this is faster :-)
	for tarball in "${distdir}/${module}-"[0-9]*.tar.{gz,bz2}
	do
	    if [ ! -f "${tarball}" ]
	    then
		continue
	    fi
	    cmd cd "${srcdir}"
	    base="$(basename ${tarball} .tar.gz)"
	    base="$(basename ${base} .tar.bz2)"
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
	    case "${tarball}" in
		*.tar.gz)
		    cmd tar xvfz "${tarball}"
		    ;;
		*.tar.bz2)
		    cmd tar xvfj "${tarball}"
		    ;;
	    esac
	    cmd cd "${srcdir}/${base}"
	    cmd ./configure --prefix="${instroot}"
	    cmd make install
	    echo -e "${instroot}\nRemove this file if you want gphoto-meta to rebuild this package" > installed-yet
	done
    done < "${cvsmodulelist}"

    echo
    echo "##########################################################################"
    echo "# Installed everything into ${instroot}"
    echo "# You may want to set the following variables to use the gphoto suite"
    echo "# installed there:"
    echo "##########################################################################"
    echo "export PATH=\"${instroot}/bin:${PATH}\""
    echo "export LD_LIBRARY_PATH=\"${instroot}/lib:${LD_LIBRARY_PATH}\""
    echo "export PKG_CONFIG_PATH=\"${instroot}/lib/pkgconfig:${PKG_CONFIG_PATH}\""
    echo "########################################################################"
}

compileinstall
