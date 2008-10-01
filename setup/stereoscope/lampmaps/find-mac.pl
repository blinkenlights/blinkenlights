#!/usr/bin/perl

while (1) {
	print STDERR "waiting for barcode ...\n";
	$barcode = <>;

	next if not ($barcode =~ m/^MAC (....) (..)$/);
	$mac = $1;
	$_ = `grep -wn $mac txt/lampmap*`;
	
	if (!($_ =~ m,^txt/lampmap-([0-9]*)([ew]).*:([0-9]*):MAC (.*) ,))
		{ print STDERR "OOPS, barcode not found!?\n"; next; }
	
	$floor = $1; $ew = $2; $id = $3; $mac = $4;
		
	if ($ew eq "e") 
		{ $floor += 100 };

	print "id $floor $id\n";
}

