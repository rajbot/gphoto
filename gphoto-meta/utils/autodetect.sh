
########################################################################
# detect automake

for suffix in -1.6 1.6 -1.5 1.5 ""
do
    if "automake${suffix}" "--version" | grep -qi "automake"
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

########################################################################
# detect autoconf

for suffix in -2.53 2.53 -2.50 2.50 ""
do
    if "autoconf${suffix}" "--version" | grep -qi "autoconf"
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
