#!/bin/sh

umount /mnt

dd if=/dev/zero of=flash.img bs=1024 count=1856
mkfs.minix -i 32 -n 14 flash.img
mount -o loop -t minix flash.img /mnt
cp -p boot.b /mnt
cp -p rimage.gz /mnt
cp -p zimage /mnt
sync
./lilo -C flash.cfg
df /mnt
sync
umount /mnt
cat preload.bin flash.img >dnpx.img
ls -al dnpx.img

