#!/bin/sh

dest=/var/www/

for n in $(seq 1 8); do
	dir="23.23.23.$n"
	echo -n "Packing update file for $dir..."
	cd $dir
	tar -f $dest/update-$dir.tar -c *
	cd ..
	echo "done."
done

echo -n "Packing generic update file for all MCUs..."
cd update
tar -f $dest/update.tar -c *
echo "done."
