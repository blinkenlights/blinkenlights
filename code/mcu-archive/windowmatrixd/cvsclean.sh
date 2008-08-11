#!/bin/sh
CWD=`pwd`

[ -f Makefile ] && make clean

find "${CWD}" -type f -name \.cvsignore | while read CVSIGNORE; do
    grep "/" "${CVSIGNORE}" && exit 23
    (while read FILE; do
        if [ "${FILE}" ]; then
	    REMOVE="${CVSIGNORE%.cvsignore}${FILE}"
	    [ -e "${REMOVE}" ] && rm -vrf "${REMOVE}"
	fi
    done) < "${CVSIGNORE}"
done

exit 0
