#!/bin/sh

# make-xml.sh
#
# recursively walk down the blm archive and
# generate XML-BLM for every movie
#
# Usage:
#		sh make-preview.sh [xmldir]

if [ x$1x != xx ]
then
	xmldir=$1
else
	xmldir=xml
fi

rm -rf $xmldir
find . -type f -name '*.blm' -print | grep -v CVS | sed 's;^\./;;'  >.blmpaths

for blmpath in `cat .blmpaths`
do
	blmdir=`dirname $blmpath`
	blmfile=`basename $blmpath`
	blmbase=`echo $blmfile | sed s/.blm$//`

	if [ ! -d $xmldir/$blmdir ]
	then
		echo Making $xmldir/$blmdir
		mkdir -p $xmldir/$blmdir
	fi

	echo "blm2xml $blmpath >$xmldir/${blmdir}/${blmfile}.xml"
	blm2xml $blmpath >$xmldir/${blmdir}/${blmfile}.xml
done

rm .blmpaths
