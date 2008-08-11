#!/bin/sh
#
# file-index.sh
#
# Usage: file-index.sh dir extension document-element file-element

dir=$1
extension=$2
document_element=$3
file_element=$4

(
	cd $dir
	echo "<?xml version=\"1.0\"?>"
	echo "<$document_element>"
	for file in *.$extension
	do
		file_id=`echo $file | sed s/\\.$extension\$//`
		echo "	<$file_element id=\""$file_id\""/>"
	done
	echo "</$document_element>"
)
