this="$(basename $0)"

metadir="$(dirname $0)"
if [ "$metadir" = "." ]
then
    metadir="${PWD}"
fi

cvsmodulelist="${metadir}/cvs-module-list"

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

cmd mkdir -p "${tmpdir}"
cmd cd "${tmpdir}"

cmd cd "${metadir}"
