this="$(basename $0)"

metadir="$(dirname $0)"
if [ "$metadir" = "." ]
then
    metadir="${PWD}"
fi

cvsmodulelistsource="${metadir}/cvs-module-list"
cvsmodulelist="${metadir}/cvs-module-list.filtered"

cvsorig="${metadir}/cvs-orig"
cvssrc="${metadir}/cvs-src"
distdir="${metadir}/dist-files"
distroot="${metadir}/dist-root"
srcdir="${metadir}/src"
instroot="${metadir}/inst-root"
downloads="${metadir}/downloads"
toolroot="${metadir}/tool-root"
toolsrc="${metadir}/tool-src"
tmpdir="${metadir}/tmp"

fail() {
    echo "${@}"
    exit 1
}

cmd() {
    echo "#> ${@}" >&2
    if [ "$1" = "cd" ]
    then
	"${@}"
    else
	nice "${@}"
    fi
    status=$?
    if [ $status -ne 0 ]
    then
	fail
    fi
}

if [ "$cvsmodulelistsource" -nt "$cvsmodulelist" ]
then
    echo "Re-creating $cvsmodulelist from $cvsmodulelistsource"
    grep -v '^#' < "$cvsmodulelistsource" | grep -v "^$" > "$cvsmodulelist"
fi

if bzip2 --version < /dev/null > /dev/null 2>&1
then
	echo "bzip2 found."
	compression="bz2"
else
	echo "bzip2 not found."
	compression="gz"
fi
sed -e "s/\\\${compression}/${compression}/g" < build-tool-list > build-tool-list.boot
buildtoollist="build-tool-list.boot"

if gmake --version< /dev/null > /dev/null 2>&1
then
	MAKE=gmake
else
	MAKE=make
fi

cmd mkdir -p "${tmpdir}"
cmd cd "${tmpdir}"

cmd cd "${metadir}"
