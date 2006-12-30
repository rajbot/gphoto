#!/bin/sh
#
# Source this file to use it.

# these are global variables
PATH="${toolroot}/bin:${PATH}"
LD_LIBRARY_PATH="${toolroot}/lib:${LD_LIBRARY_PATH}"
PKG_CONFIG_PATH="/usr/lib/pkgconfig"
export PATH LD_LIBRARY_PATH PKG_CONFIG_PATH


########################################################################
# evaluate command line parameters

parsecommandline() {
    parm_update="false"
    parm_tools="false"
    check="false"
    build_docs="false"
    while :
    do
	case "$1" in
	    --update) parm_update="true" ;;
	    --tools)  parm_tools="true"  ;;
	    --check)  check="true"       ;;
	    --docs)   build_docs="true"  ;;
	    --help)   cat<<EOF
Usage: $this [OPTION]...
Bootstrap the gphoto-meta package, i.e. download and build distribution 
tarballs of all gphoto and related packages.

     --update    run a CVS update if a module is already checked out
     --tools     use a local copy of the build tools (i.e. autoconf, 
                 automake, libtool, gettext), download and install them
                 if required
     --check     run "make distcheck" instead of "make dist"
     --docs      build documentation
EOF
		exit
		;;
	    "")       ;;
	    *)
		echo "Fatal: Unknown parameter: $1"
		exit 1
		;;
	esac
	if test $# = 0
	then
	    break
	fi
	shift
    done
    set | grep '^parm_'
}


########################################################################
# don't rely on external true and false

true() {
    return 0
}

false() {
    return 1
}


########################################################################
# installautotools - download, build and locally install build tools

installautotools() {
    local tool URL restofline

    cmd mkdir -p "${downloads}" "${toolroot}" "${toolsrc}"
    #cmd rm -rf "${toolroot}"

    while read tool action URL restofline
    do
    	if test "x$action" = "xauto"; then
	    if test -x "${toolroot}/bin/${tool}"; then
	    	echo "##### $tool already build. Skipping."
		continue
	    fi
	fi
	case "$URL" in
	    http://*) ;;
	    ftp://*)  ;;
	    *)        continue ;;
	esac
	local tarball
	tarball="${downloads}/$(basename "$URL")"
	if test -f "$tarball"
	then
	    echo "##### $tool already downloaded. Skipping."
	else
	    cmd cd "${downloads}"
	    if ! cmd wget --continue "$URL"
	    then
		echo "##### Fatal: Could not get $tool from $URL."
		echo "    # You may want to manually download $URL to $downloads."
		exit 12
	    fi
	fi
	echo "    # Distribution tarball is at $tarball."
	cmd cd "$toolsrc"
	cmd rm -rf "$toolsrc/$tool-"[0-9]*
	local ext
	case "$tarball" in
	    *.tar.gz)
		cmd tar xvfz "$tarball"
		ext=".tar.gz"
		;;
	    *.tar.bz2)
		cmd bzip2 -d -c "$tarball" | cmd tar xvf -
		ext=".tar.bz2"
		;;
	    *)
		echo "#### Fatal: Unknown format of $tarball."
		exit 124
		;;
	esac
	base="$(basename "$tarball" "$ext")"
	cmd cd "${toolsrc}/${base}"
	cmd ./configure --prefix="${toolroot}" --disable-csharp
	if "$check" && test "$tool" != "gettext"; then cmd "$MAKE" $MAKEOPT check; fi
	cmd "$MAKE" $MAKEOPT install
    done < "${buildtoollist}"
    (cd "${toolroot}/bin" && patch -p0 < "${metadir}/gettextize.patch")
}


########################################################################
# checktools - check for necessary tools to be installed

checktools() {
    local action tool restofline
    local autoinstall=false

    echo "##### Checking for presence of tools..."
    while read tool action restofline
    do
    	if test "x$action" = 'xauto'; then
	    	tp="$toolroot/bin/$tool"
		if test -x "$tp"; then
			echo "    # $tool found."
			continue;
		fi
	else
		tp="$tool"
	fi
	echo "$tp" --version
	output=$("$tp" --version < /dev/null)
	if [ $? -eq 0 ]
	then
	    if [ "$tool" = "pkg-config" ] || echo "$output" | grep -i "$tool" > /dev/null
	    then
		echo "    # $tool found."
		continue
	    fi
	fi
	echo "    # $tool not found."
	if test "$action" = "fatal" || ( test "$action" = "autocnd" && test "$autoinstall" = "true" )
	then
	    echo "    # $tool is absolutely required, so we're aborting."
	    exit 2
	elif [ "$action" = "auto" ]
	then
	    echo "    # $tool is required, so we're installing the autosuite first."
	    autoinstall="true"
	else
	    echo "##### Illegal action. Internal error."
	    exit 225
	fi
    done < "${buildtoollist}"

    if "${autoinstall}"
    then
	installautotools
    elif "$parm_tools"
    then
	if [ -d "$toolroot" ]
	then
	    echo "##### Going to use/build our own tools from/to ${toolroot}"
	    echo "    # If you don't want to use these tools, remove the"
	    echo "    # ${toolroot} directory."
	else
	    installautotools
	fi
    else
	echo "##### Going to use autodetected tools from the system"
    fi

    # no need for autodetection if we installed our own version
    [ -d "$toolroot" ] && return

    for suffix in -1.6 1.6 -1.5 1.5 ""
    do
	if "automake${suffix}" "--version" 2> /dev/null | grep -qi "automake"
	then
	    echo "Detected automake: automake${suffix}"
	    AUTOMAKE_SUFFIX="$suffix"
	    break
	fi
    done
    if ! set | grep -q AUTOMAKE_SUFFIX
    then
	echo "$this: Fatal: automake not found"
	exit 2
    fi
    export AUTOMAKE_SUFFIX

    for suffix in -2.53 2.53 -2.50 2.50 ""
    do
	if "autoconf${suffix}" "--version" 2> /dev/null | grep -qi "autoconf"
	then
	    echo "Detected autoconf: autoconf${suffix}"
	    AUTOCONF_SUFFIX="$suffix"
	    break
	fi
    done
    if ! set | grep -q AUTOCONF_SUFFIX
    then
	echo "$this: Fatal: autoconf not found"
	exit 2
    fi
    export AUTOCONF_SUFFIX

    echo "AUTOCONF_SUFFIX=$AUTOCONF_SUFFIX"
    echo "AUTOMAKE_SUFFIX=$AUTOMAKE_SUFFIX"
}


########################################################################
# cvslogin - initialize stuff (CVS logins, etc)

cvslogin() {
    local module releasetag CVSROOT restofline
    while read module releasetag CVSROOT restofline
    do 
	CVSROOT="$(echo "${CVSROOT}" | sed 's|:/|:2401/|')"
	if grep -q "${CVSROOT}" $HOME/.cvspass
	then
	    echo "Good: already logged in for ${CVSROOT}."
	    continue
	fi
	echo "##### Logging in for ${CVSROOT}."
	echo "# Just press enter when asked for the CVS password:"
	cmd cvs -d "${CVSROOT}" login
    done < "${cvsmodulelist}"
}


########################################################################
# getsources - get software sources from CVS into ${cvsorig}/MODULE

getsources() {
    echo "##### Getting software from CVS"
    cmd mkdir -p "${cvsorig}"
    cmd cd "${cvsorig}"
    local module releasetag CVSROOT restofline
    while read module releasetag CVSROOT restofline
    do 
	local releaseparm
	if [ 'HEAD' = "$releasetag" ]
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
		echo "    # Updating ${module}"
		cmd cd "${cvsorig}/${module}"
		# FIXME: Use other directory (do not modify timestamps if not required)
		local stdout="${tmpdir}/.tmp.${module}.stdout"
		cmd rm -f "$stdout"
		xmd cvs -z3 update -d -P ${releaseparm} > "$stdout"
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
		echo "    # Not updating ${module} - run $this with --update if you want that."
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
    local PKG_CONFIG_PATH="${distroot}/lib/pkgconfig:${PKG_CONFIG_PATH}"
    local LD_LIBRARY_PATH="${distroot}/lib:${LD_LIBRARY_PATH}"
    local PATH="${distroot}/bin:${PATH}"
    export PKG_CONFIG_PATH LD_LIBRARY_PATH PATH
    echo "##### USING PATH=$PATH FOR BUILDING DIST TARBALLS"
    cmd mkdir -p "${cvssrc}"
    cmd mkdir -p "${distdir}"
    while read module releasetag CVSROOT distopts configopts
    do
	local redist="true"
	local file
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
	    echo "    # Not rebuilding dist tarball for ${module}."
	else
	    if [ "$module" = "gphoto2-manual" ] && ! xmlto --version > /dev/null 2> /dev/null
	    then
		echo "##### skipping gphoto2-manual - requires xmlto, which is not installed"
		continue
	    fi
	    echo "########################################################################"
	    echo "##### Creating new distribution tarball of ${module} now:"
	    echo "#        releasetag:  $releasetag"
	    echo "#        distopts:    $distopts"
	    echo "#        configopts:  $configopts"
	    echo "########################################################################"
	    chmod -R +w "${cvssrc}/${module}"
	    cmd rm -rf "${cvssrc}/${module}"
	    cmd cp -R "${cvsorig}/${module}" "${cvssrc}/"
	    cmd cd "${cvssrc}/${module}"

	    # hack: this directory shouldn't be in CVS
	    # FIXME: shouldn't we just use "cvs checkout -P" ?
	    [ "$module" = "exif" ] && [ -d exif/exif ] && cmd rm -rf exif/exif
	    # hack: gnocam is called GnoCam - or the other way round
	    [ "$module" = "gnocam" ] && module="GnoCam"

	    # hack: try to copy man pages generated by gphoto2-manual to the source
	    local docroot
	    local dir
	    docroot="${distroot}/share/doc/gphoto2-manual"
	    for dir in "${distroot}/share/doc/gphoto2-manual"*
	    do
		if [ -d "$dir" ]
		then
		    docroot="$dir"
		fi
	    done

	    if [ -d "$docroot" ]
	    then
		echo "##### Installing documentation for ${module}..."
		case "${module}" in
		    gtkam)	   
			cp -f "${docroot}/man/gtkam.1" doc/gtkam.1
			;;
		    gphoto2)	   
			cp -f "${docroot}/man/gphoto2.1" doc/gphoto2.1
			;;
		    libgphoto2)	   
			cp -f "${docroot}/man/libgphoto2.3" doc/libgphoto2.3
			cp -f "${docroot}/man/libgphoto2_port.3" doc/libgphoto2_port.3
			;;
		esac
	    else
		echo "##### \"$docroot\" is no directory. -> Not installing generated man pages"
		mkdir -p "$docroot"
	    fi

	    # special config options for documentation
	    if "${build_docs}"
	    then
	    	case "${module}" in
		    libgphoto2)
			configopts="$configopts --enable-docs --with-html-dir=${docroot}/api-html"
			;;
		    gphoto2-manual)
			configopts="$configopts --with-doc-formats=man,html,txt,pdf"
			;;
		esac
	    fi

	    echo "##### Press enter when asked to. And complain to the gettextize guys,"
	    echo "    # not to me. Or run this with \"echo $0 | at now\"."
	    if xmd ./autogen.sh --enable-maintainer-mode --prefix="${distroot}" ${configopts}; then :; else
	    	cmd ./autogen.sh --init
	    fi
	    cmd ./configure  --enable-maintainer-mode --prefix="${distroot}" ${configopts}
	    target="dist"
	    if "$check"; then target="distcheck"; fi
	    cmd "$MAKE" $MAKEOPT "$target"

	    # create .tar.bz2 source tarball if possible and not done yet
	    if [ "${compression}" = "bz2" ]
	    then
		echo "##### creating .bz2 tarball from .gz tarball"
		for f in "${module}-"[0-9]*.tar.gz
		do
		    local b		  
		    b="$(basename "${f}" ".gz").bz2"
		    echo "    # examining \"$f\" and \"$b\" resp."
		    if [ -s "$f" ] && [ ! -s "$b" ]
		    then
		    echo "    # converting \"$f\" to \"$b\""
			gunzip -c "$f" | bzip2 -c > "$b"
		    fi
		done
		cmd mv "${module}-"[0-9]*.tar.bz2 "${distdir}/"
	    fi

	    # move source tarballs to distribution files
	    cmd mv "${module}-"[0-9]*.tar.gz "${distdir}/"

	    if [ "$distopts" = "install" ]
	    then
		# install if required
		cmd "$MAKE" $MAKEOPT install
	    else
		# try to install even if not required
		if ! "$MAKE" install
		then
		    # mark dist package as broken
		    local file
		    for file in "${distdir}/${module}-"[0-9]*.tar.{bz2,gz}
		    do
			local ext
			if [ -f "$file" ] && ! echo "$file" | egrep -q -- '-broken.tar.(bz2|gz)$'
			then
			    case "$file" in
				*.tar.bz2)
				    ext=".tar.bz2"
				    ;;
				*.tar.gz)
				    ext=".tar.gz"
				    ;;
				*)
				    echo "Unknown extension: $file"
				    exit 1
				    ;;
			    esac
			    newname="$(basename "$file" "$ext")-broken"
			    cmd mv "$file" "${distdir}/${newname}${ext}"
			fi
		    done
		fi
	    fi

	    if [ "$module" = "gphoto2-manual" ]
	    then
		echo "##### building special manual packages"
		local docroot
		local dir
		for dir in "${distroot}/share/doc/gphoto2-manual"*
		do
		    if [ -d "$dir" ]
		    then
			docroot="$dir"
		    fi
		done
		for dir in html xhtml html-nochunks xhtml-nochunks man pdf txt
		do
		    ls "${docroot}/${dir}/"*
		    if ls "${docroot}/${dir}/"* > /dev/null 2>&1
		    then
			echo "    # building special manual package for ${dir}"
			local version
			version="$(grep '^AM_INIT_AUTOMAKE' configure.in | sed -e 's/^[^,]*, *//g' | sed -e 's/[,)].*//g')"
			local base
			base="${distdir}/${module}-${dir}-${version}.tar"
			(cd "${docroot}" && tar cvfhz "${base}.gz" "${dir}/")
			if [ "$compression" = "bz2" ]
			then
			    gunzip -c "${base}.gz" | bzip2 -c > "${base}.bz2"
			fi
		    fi
		done
	    fi

	    # make sure time stamps for dependencies are correct
	    local file
	    for file in "${distdir}/${module}-"[0-9]*.tar.{gz,bz2}
	    do
		[ -f "$file" ] && cmd touch "$file"
	    done
	fi
    done < "${cvsmodulelist}"

    # create list of supported cameras
    (
	echo "========================================================================"
	TZ=UTC date
	echo "========================================================================"
	gphoto2 --version
        echo "========================================================================"
	gphoto2 --list-cameras
	echo "========================================================================"
    ) > "${distdir}/SUPPORTED-CAMERAS"

    echo "##########################################################################"
    echo "# Installed test versions of everything into ${distroot}"
    echo "# You may want to set the following variables to use the gphoto suite"
    echo "# installed there:"
    echo "##########################################################################"
    echo "export PATH=\"${PATH}\""
    echo "export LD_LIBRARY_PATH=\"${LD_LIBRARY_PATH}\""
    echo "export PKG_CONFIG_PATH=\"${PKG_CONFIG_PATH}\""
    echo "########################################################################"
}


########################################################################
# makefiles - create Makefile.am files

makefiles() {
    cmd cd "${distdir}"
    local files=""
    while read module restofline
    do
	local tarball
	# hack: gnocam is called GnoCam - or the other way round
	[ "$module" = "gnocam" ] && module="GnoCam"
	# add bz2 to dist if available, otherwise gz
	for tarball in "${distdir}/${module}-"[0-9]*.tar.{bz2,gz}
	do
	    if [ -s "${tarball}" ]
	    then
		files="${files} $(basename ${tarball})"
		if [ "$module" = "gphoto2-manual" ]
		then
		    case "${tarball}" in
			*.bz2) ext=".bz2" ;;
			*.gz)  ext=".gz" ;;
			*)     ext="" ;;
		    esac
		    echo "ext=$ext"
		    # add bz2 to dist if available, otherwise gz
		    for tarball in "${distdir}/${module}-html-"[0-9]*.tar${ext} "${distdir}/${module}-html.tar${ext}"
		    do
			if [ -s "${tarball}" ]
			then
			    files="${files} $(basename ${tarball})"
			    break
			fi
		    done
		fi
		tarball=""
		break
	    fi
	done
	[ "$tarball" != "" ] && echo "##### Warning: No tarball for ${module}"
    done < "${cvsmodulelist}"
    cd "$distdir" && md5sum *.tar.* > MD5SUMS
    files="${files} MD5SUMS"
    cat > "${distdir}/Makefile.am" <<EOF
# This file is autogenerated by $this. DO NOT MODIFY.

EXTRA_DIST = ${files} 
EOF
}


########################################################################
# main program

die() {
    echo "$this: Fatal error. Dying."
    exit 13
}
