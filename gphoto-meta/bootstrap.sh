#!/bin/bash

source "$(dirname $0)/utils/common.sh" || exit 7


########################################################################
# evaluate command line parameters

parm_update="false"
parm_tools="false"
while :
do
    case "$1" in
	--update) parm_update="true" ;;
	--tools)  parm_tools="true" ;;
	--help)   cat<<EOF
Usage: $this [OPTION]...
Bootstrap the gphoto-meta package, i.e. download and build distribution 
tarballs of all gphoto and related packages.

     --update    run a CVS update if a module is already checked out
     --tools     use a local copy of the build tools (i.e. autoconf, 
                 automake, libtool, gettext), download and install them
                 if required
EOF
		  exit
		  ;;
	"")       ;;
	*)
	    echo "Fatal: Unknown parameter: $1"
	    exit 1
	    ;;
    esac
    if ! shift
    then
	break
    fi
done
set | grep '^parm_'


########################################################################
# installautotools - check for necessary tools to be installed

function installautotools() {
    local tool URL restofline

    cmd mkdir -p "${downloads}" "${toolroot}" "${toolsrc}"
    cmd rm -rf "${toolroot}"

    while read tool URL restofline
    do
	local tarball
	tarball="${downloads}/$(basename "$URL")"
	if [ -f "$tarball" ]
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
		cmd tar xvfj "$tarball"
		ext=".tar.bz2"
		;;
	    *)
		echo "#### Fatal: Unknown format of $tarball."
		exit 124
		;;
	esac
	base="$(basename "$tarball" "$ext")"
	cmd cd "${toolsrc}/${base}"
	cmd ./configure --prefix="${toolroot}"
	cmd make install
    done <<EOF
autoconf	ftp://ftp.gnu.org/gnu/autoconf/autoconf-2.53.tar.bz2
automake	ftp://ftp.gnu.org/gnu/automake/automake-1.6.3.tar.bz2
libtool		ftp://ftp.gnu.org/gnu/libtool/libtool-1.4.2.tar.gz
gettext		ftp://ftp.gnu.org/gnu/gettext/gettext-0.11.5.tar.gz
EOF
}


########################################################################
# checktools - check for necessary tools to be installed

function checktools() {
    local action tool restofline
    local autoinstall="false"

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

    echo "##### Checking for presence of tools..."
    while read action tool restofline
    do
	if "$tool" --version >& /dev/null
	then
	    echo "    # $tool found."
	else
	    echo "    # $tool not found."
	    if [ "$action" = "fatal" ] || ( [ "$action" = "autocnd" ] && [ "$autoinstall" = "true" ] )
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
	fi
    done <<EOF
fatal	cvs
fatal	grep
fatal	egrep
auto	autoconf
auto	automake
auto	gettext
auto	libtool
autocnd	wget
EOF

    if [ "${parm_tools}" = "false" ]
    then
	true
    elif [ "${autoinstall}" = "false"�]
    then
	true
    else
	installautotools
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
    if [ "$AUTOMAKE_SUFFIX" = "" ]
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
    if [ "$AUTOCONF_SUFFIX" = "" ]
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

function cvslogin() {
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

function getsources() {
    echo "##### Getting software from CVS"
    cmd mkdir -p "${cvsorig}"
    cmd cd "${cvsorig}"
    local module releasetag CVSROOT restofline
    while read module releasetag CVSROOT restofline
    do 
	local releaseparm
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
		echo "    # Updating ${module}"
		cmd cd "${cvsorig}/${module}"
		# FIXME: Use other directory (do not modify timestamps if not required)
		local stdout="${tmpdir}/.tmp.${module}.stdout"
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

function builddist() {
    local PKG_CONFIG_PATH="${distroot}/lib/pkgconfig:${PKG_CONFIG_PATH}"
    local LD_LIBRARY_PATH="${distroot}/lib:${LD_LIBRARY_PATH}"
    local PATH="${distroot}/bin:${PATH}"
    export PKG_CONFIG_PATH LD_LIBRARY_PATH PATH
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
	    local docroot="${distroot}/share/doc/gphoto2-manual-"[0-9]*
	    if [ -d "$docroot" ]
	    then
		echo "##### Installing documentation for ${module}..."
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
	    echo "##### Press enter when asked to. And complain to the gettextize guys,"
	    echo "    # not to me. Or run this with \"echo $0 | at now\"."
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
	    local file
	    for file in "${distdir}/${module}-"[0-9]*.tar.{gz,bz2}
	    do
		[ -f "$file" ] && cmd touch "$file"
	    done
	fi
    done < "${cvsmodulelist}"
    ( TZ=UTC date; echo; gphoto2 --version; echo; gphoto2 --list-cameras ) \
	> "${distdir}/SUPPORTED-CAMERAS"
}


########################################################################
# makefiles - create Makefile.am files

function makefiles() {
    cmd cd "${distdir}"
    local files=""
    while read module restofline
    do
	local tarball
	# add bz2 to dist if available, otherwise gz
	for tarball in "${distdir}/${module}-"[0-9]*.tar.{bz2,gz}
	do
	    if [ -s "${tarball}" ]
	    then
		files="${files} $(basename ${tarball})"
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

checktools
cvslogin
getsources
builddist
makefiles
