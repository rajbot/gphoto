#!/bin/sh
#
# Unchecked dependencies:
# - build tool: pkg-config required
# - exif, gphoto2 requires libpopt, rest can be built without it
# - libexif-gtk, gexif, gtkam require GTK 2.0, rest can be built without it
# - 

. "$(dirname $0)/utils/common.sh"

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

# FIXME hack. we have to determine default PKG_CONFIG_PATH somehow.
if [ "x$PKG_CONFIG_PATH" = "x" ]
then
    PKG_CONFIG_PATH=/usr/lib/pkgconfig
fi
export PKG_CONFIG_PATH

########################################################################
# compile & install stuff

compileinstall() {
    local ext bool
    bool=true
    for ext in .tar.gz .tar.bz2
    do
	if ! ls "${distdir}"/*${ext} > /dev/null 2>&1
	then
	    echo "$0: No *${ext} source distributions found."
	else
	    echo "$0: *${ext} source distributions found."
	    bool=false
	    break
	fi
    done

    if [ "$bool" = "true" ]
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
    while read module restofline
    do
	# FIXME: Hack. gphoto2-manual requires relatively exotic tools.
	#if [ "$module" = "gphoto2-manual" ]
	#then
	#    continue
	#fi
	# unpack gz if available, else bz2 (this is faster :-)
	for ext in gz bz2
	do
	for tarball in "${distdir}/${module}-"[0-9]*.tar.$ext
	do
	    if [ ! -f "${tarball}" ]
	    then
		continue
	    fi
	    cmd cd "${srcdir}"
	    base="$(basename ${tarball} .tar.gz)"
	    base="$(basename ${base} .tar.bz2)"
	    base="$(basename ${base} -broken)"
	    if [ -f "${base}/installed-yet" ]
	    then
		read dir < "${base}/installed-yet"
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
		    cmd bzip2 -d -c "${tarball}" | tar xvf  -
		    ;;
	    esac
	    cmd cd "${srcdir}/${base}"
	    cmd ./configure --prefix="${instroot}"
	    cmd "$MAKE" install
	    echo -e "${instroot}\nRemove this file if you want gphoto-meta to rebuild this package" > installed-yet
	done
	done
    done < "${cvsmodulelist}"

    echo
    echo "##########################################################################"
    echo "# Installed everything into ${instroot}"
    echo "# You may want to set the following variables to use the gphoto suite"
    echo "# installed there:"
    echo "##########################################################################"
    echo "export PATH=\"${PATH}\""
    echo "export LD_LIBRARY_PATH=\"${LD_LIBRARY_PATH}\""
    echo "export PKG_CONFIG_PATH=\"${PKG_CONFIG_PATH}\""
    echo "########################################################################"
}

compileinstall
