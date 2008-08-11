#! /usr/bin/env bash
cd /dev
rm -f pio*

mknod pio0  c 60  0
mknod pio1  c 60  1
mknod pio2  c 60  2

mknod pio3  c 60  3
mknod pio4  c 60  4
mknod pio5  c 60  5

mknod pio6  c 60  6
mknod pio7  c 60  7
mknod pio8  c 60  8

mknod pio9  c 60  9
mknod pio10 c 60 10
mknod pio11 c 60 11

mknod pio12 c 60 12
mknod pio13 c 60 13
mknod pio14 c 60 14

mknod pio15 c 60 15
mknod pio16 c 60 16
mknod pio17 c 60 17
