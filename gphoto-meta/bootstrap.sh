#!/bin/bash

source "$(dirname $0)/utils/common.sh" || exit 7
source "${metadir}/utils/autodetect.sh" || exit 7

########################################################################
# evaluate command line parameters

if [ "$1" = "--update" ]
then
    parm_update="true"
else
    parm_update="false"
fi

########################################################################
# cvslogin - initialize stuff (CVS logins, etc)

cvslogin() {
    while read module release CVSROOT restofline
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
# getsources - get software sources from CVS into ${cvsorig}/MODULE

getsources() {
    echo "##### Getting software from CVS"
    cmd mkdir -p "${cvsorig}"
    cmd cd "${cvsorig}"
    while read module releasetag CVSROOT restofline
    do 
	if [ 'HEAD' = $releasetag ]
	then
	    releaseparm=""
	else
	    case "$releasetag" in
		-*)
		    releaseparm="$releasetag"
		    ;;
		*)
		    releaseparm="-r${releasetag}"
	    esac
	fi
	if [ -d "${cvsorig}/${module}" ]
	then
	    echo "##### CVS module ${module} is already here."
	    if [ "$parm_update" = "true" ]
	    then
		echo "#     Updating ${module}"
		cmd cd "${cvsorig}/${module}"
		# FIXME: Use other directory (do not modify timestamps if not required)
		stdout="${tmpdir}/.tmp.${module}.stdout"
		cmd rm -f "$stdout"
		cmd cvs -z3 update -d -P > "$stdout"
		if [ -s "$stdout" ]
		then
		    # FIXME: Handle conflicts, etc.
		    echo "##### Module ${module} was updated."
		    cmd touch "${cvsorig}/.stamp.${module}"
		else
		    echo "##### Module ${module} was already current."
		fi
		rm -f "$stdout"
	    else
		echo "#     Not updating ${module} - run $this with --update if you want that."
	    fi
	else
	    echo "##### Checking out ${module} release ${releasetag} from ${CVSROOT}"
	    cmd cvs -z3 -d "${CVSROOT}" checkout ${releaseparm} "${module}"
	    cmd touch "${cvsorig}/.stamp.${module}"
	fi
    done < "${cvsmodulelist}"
}


########################################################################
# builddist - build distribution tarball for all MODULEs

builddist() {
    export PKG_CONFIG_PATH="${distroot}/lib/pkgconfig:${PKG_CONFIG_PATH}"
    export LD_LIBRARY_PATH="${distroot}/lib:${LD_LIBRARY_PATH}"
    export PATH="${distroot}/bin:${PATH}"
    cmd mkdir -p "${cvssrc}"
    cmd mkdir -p "${distdir}"
    while read module releasetag CVSROOT distopts configopts
    do
	redist="true"
	for file in "${distdir}/${module}-"[0-9]*.tar.{bz2,gz}
	do 
	    if [ -f "$file" ] && [ ! "${file}" -ot "${cvsorig}/.stamp.${module}" ]
	    then
		redist="false"
		break
	    fi
	done
	if [ "$redist" != "true" ]
	then
	    echo "##### Distribution tarball ${file} for ${module} is current."
	    echo "#     Not rebuilding dist tarball for ${module}."
	else
	    echo "########################################################################"
	    echo "##### Creating new distribution tarball of ${module} now:"
	    echo "#        releasetag:  $releasetag"
	    echo "#        distopts:    $distopts"
	    echo "#        configopts:  $configopts"
	    echo "########################################################################"
	    cmd rm -rf "${cvssrc}/${module}"
	    # FIXME: relies on GNU cp
	    cmd cp -a "${cvsorig}/${module}" "${cvssrc}/"
	    cmd cd "${cvssrc}/${module}"
	    docroot="${distroot}/share/doc/gphoto2-manual-"[0-9]*
	    if [ -d "$docroot" ]
	    then
		echo "#### Installing documentation for ${module}..."
		case "${module}" in
		    gtkam)	   
			cmd cp -f "${docroot}/man/gtkam.1" doc/gtkam.1
			;;
		    gphoto2)	   
			cmd cp -f "${docroot}/man/gphoto2.1" doc/gphoto2.1
			;;
		    libgphoto2)	   
			cmd cp -f "${docroot}/man/gphoto2.3" doc/gphoto2.3
			cmd cp -f "${docroot}/man/gphoto2_port.3" doc/gphoto2_port.3
			;;
		esac
	    fi
	    echo "#### Press enter when asked to. And complain to the gettextize guys,"
	    echo "#    not to me. Or run this with \"echo $0 | at now\"."
	    cmd ./autogen.sh --enable-maintainer-mode --prefix="${distroot}" ${configopts}
	    cmd ./configure  --enable-maintainer-mode --prefix="${distroot}" ${configopts}
	    cmd make dist

	    cmd mv "${module}-"[0-9]*.tar.gz "${distdir}/"
	    [ -s "${module}-"[0-9]*.tar.bz2 ] && cmd mv "${module}-"[0-9]*.tar.bz2 "${distdir}/"

	    if [ "$distopts" = "install" ]
	    then
		cmd make install
	    else
		if ! make install
		then
		    for f in "${distdir}/${module}-"[0-9]*.tar.{bz2,gz}
		    do
			if [ -f "$f" ] && ! echo "$f" | egrep -q -- '-broken.tar.(bz2|gz)$'
			then
			    case "$f" in
				*.tar.bz2)
				    ext=".tar.bz2"
				    ;;
				*.tar.gz)
				    ext=".tar.gz"
				    ;;
				*)
				    echo "Unknown extension: $f"
				    exit 1
				    ;;
			    esac
			    newname="$(basename "$f" "$ext")-broken"
			    cmd mv "$f" "${distdir}/${newname}${ext}"
			fi
		    done
		fi
	    fi
	    for file in "${distdir}/${module}-"[0-9]*.tar.{gz,bz2}
	    do
		[ -f "$file" ] && cmd touch "$file"
	    done
	fi
    done < "${cvsmodulelist}"
    (TZ=UTC date;gphoto2 --version; gphoto2 --list-cameras) > "${distdir}/SUPPORTED-CAMERAS"
}

########################################################################
# makefiles - create Makefile.am files

makefiles() {
    cmd cd "${distdir}"
    files=""
    while read module restofline
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

cvslogin
getsources
builddist
makefiles
