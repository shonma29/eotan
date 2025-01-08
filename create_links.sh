#!/bin/sh

show_banner() {
	echo "usage: ${0##*/} -a architecure -m mpu" 1>&2
	exit 1
}

check_second_time() {
	if [ -n "$2" ]; then
		echo "Already defined -$1" 1>&2
		show_banner
	fi
}

create_link() {
	if [ -e "$2" ]; then
		echo "File exists $2" 1>&2
	else
		ln -s -v $1 $2
	fi
}

ARCH=
MPU=

# parse options
while getopts a:m: opt; do
	case ${opt} in
	a)
		check_second_time a ${ARCH}
		ARCH=${OPTARG};;
	m)
		check_second_time m ${MPU}
		MPU=${OPTARG};;
	esac
done

# unnecessary arguments
shift $((${OPTIND} - 1))
if [ $# -gt 0 ]; then
	show_banner
fi

# necessary arguments
if [ -z "${ARCH}" ]; then
	show_banner
fi

if [ -z "${MPU}" ]; then
	show_banner
fi

# check values
case ${ARCH} in
pcat)
	;;
*)
	echo "Unknown architecture ${ARCH}" 1>&2
	exit 2;;
esac

case ${MPU} in
i686)
	;;
*)
	echo "Unknown mpu ${MPU}" 1>&2
	exit 2;;
esac

# process
echo "arch = ${ARCH}"
echo "mpu = ${MPU}"

create_link ${ARCH} include/arch
create_link ${MPU} include/mpu
create_link ${ARCH} drivers/arch
create_link ${MPU} drivers/mpu
create_link ${MPU} lib/libm/mpu
create_link ${ARCH} starter/arch
