#!/bin/sh

. "$(dirname $0)/utils/common.sh" || exit 7
. "$(dirname $0)/utils/bootstrap-helpers.sh" || exit 9

# remove autogen-generated files - mkinstalldirs irritates configuring
# our packages
rm -f mkinstalldirs missing ltmain.sh install-sh depcomp configure Makefile

set -x

parsecommandline "$@" || die
checktools || die
cvslogin || die
getsources || die
builddist || die
makefiles || die

echo "Note: To be sure that the packages in ${distdir} really work, run"
echo "      $(dirname $0)/compileinstall.sh"
echo "We recommend that at least if you didn't use --check."
