#!/bin/sh
loop="loop"
while [ "$loop" = "loop" ]
do
	echo
	echo "+------------------------------------------------------------------------"
	echo "| Please select which serial port on your PC the camera is connected to |"
	echo "+------------------------------------------------------------------------"
	echo
	echo "       0. /dev/ttyS0     COM1 under DOS"
	echo "       1. /dev/ttyS1     COM2 under DOS (Default)"
	echo "       2. /dev/ttyS2     COM3 under DOS"
	echo "       3. /dev/ttyS3     COM4 under DOS"
	echo
	echo "       m. /dev/modem"
	echo
	echo "       9. Another serial port to be specified."
	echo
	echo -n "Enter Selection (Default is /dev/ttyS1) --> "
	read port
	if [ X"$port" = "X" ]
	then
		port=1
	fi
	loop="finished"
	case $port in
	0|1|2|3)	echo "#define CAMERAPORT	\"/dev/ttyS$port\"" > config.h
			;;
	m)		echo "#define CAMERAPORT	\"/dev/modem\"" > config.h
			;;
	9)		echo -n "Other serial port --> "
			read port
			echo "#define CAMERAPORT	\"$port\"" > config.h
			;;
	*)		loop="loop"
			;;
	esac
done

make clean
make
