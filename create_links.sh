#!/bin/sh
# This is free and unencumbered software released into the public domain.
#
# Anyone is free to copy, modify, publish, use, compile, sell, or
# distribute this software, either in source code form or as a compiled
# binary, for any purpose, commercial or non-commercial, and by any
# means.
#
# In jurisdictions that recognize copyright laws, the author or authors
# of this software dedicate any and all copyright interest in the
# software to the public domain. We make this dedication for the benefit
# of the public at large and to the detriment of our heirs and
# successors. We intend this dedication to be an overt act of
# relinquishment in perpetuity of all present and future rights to this
# software under copyright law.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# For more information, please refer to <http://unlicense.org/>

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
