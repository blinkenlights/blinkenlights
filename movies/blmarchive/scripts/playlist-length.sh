#!/bin/sh

playlist=$1
blmfiles=`awk '{ print $1 }' $playlist`

#echo $blmfiles

gawk '

BEGIN	{ FS="@" }
/^@/	{ total_ms = total_ms + $2 }
END		{
			count = total_ms;

			ms = count % 1000;
			count = (count - ms) / 1000;
			
			seconds = count % 60;
			count = (count - seconds) / 60;
			
			minutes = count % 60;
			count = (count - minutes) / 60;
			
			hours = count % 24;
			count = (count - hours) / 24;
			
			days = count;
			
			print "Duration (dd:hh:mm:ss.mmm):" sprintf("%02d:%02d:%02d:%02d.%03d", days, hours, minutes, seconds, ms)


		}' $blmfiles
