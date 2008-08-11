#!/bin/sh

# make-preview.sh
#
# recursively walk down the blm archive and
# generate preview gifs with blm2gif.
#
# Usage:
#		sh make-preview.sh [previewdir]

if [ x$1x != xx ]
then
	previewdir=$1
	shift
else
	previewdir=preview
fi

if [ x$1x != xx ]
then
	blmdirs=$*
else
	blmdirs=.
fi

echo Making previews for $blmdirs

#rm -rf $previewdir
find $blmdirs -type f -name '*.blm' -print | grep -v CVS | sed 's;^\./;;'  >.blmpaths

for size in small medium large huge
#for size in large medium small
do
	for blmpath in `cat .blmpaths`
	do
		blmdir=`dirname $blmpath`
		blmfile=`basename $blmpath`
		targetdir=$previewdir/$size

		if [ ! -d $targetdir/$blmdir ]
		then
			echo Making $targetdir/$blmdir
			mkdir -p $targetdir/$blmdir
			touch -am -r $blmdir $targetdir/$blmdir
		fi
		echo "blm2gif --loop --hdl-$size $blmpath | gifsicle --optimize=2 > $targetdir/${blmpath}.gif"
		blm2gif --loop --hdl-$size $blmpath | gifsicle --optimize=2 > $targetdir/${blmpath}.gif
		touch -r $blmpath $targetdir/${blmpath}.gif
	done
done

rm .blmpaths
