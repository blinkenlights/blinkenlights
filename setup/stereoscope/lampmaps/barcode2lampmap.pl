#!/usr/bin/perl
#
# MAC 004A 4A
#
use strict;

if (@ARGV != 3) {
	print "usage: <input file> <floor> <screen>\n";
}

my $file = $ARGV[0];
my $floor = $ARGV[1];
my $screen = $ARGV[2];

open BARCODE, $file or die("couldn't open input file ($file): ".$!);

my $line;
my $x = 0;
while (defined($line = <BARCODE>)) {
	chomp $line;
	my ($mac) = $line =~ /^MAC (....) ..$/;
	print "0x".lc($mac)." $screen $x $floor\n";
	$x++;
}
