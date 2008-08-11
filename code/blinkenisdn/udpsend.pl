#!/usr/bin/perl -w

use strict;
use IO::Socket;
use IO::Select;
use Time::HiRes qw(sleep);

my $localaddr = shift;
my $peeraddr = shift;

my $sel = IO::Select->new(\*STDIN);

my $sock = IO::Socket::INET->new(Proto     => 'udp',
				 LocalAddr => $localaddr);
my $socksel = IO::Select->new($sock);

my $sockout = IO::Socket::INET->new(Proto     => 'udp',
				    PeerAddr  => $peeraddr);

until(undef) {

  if($sel->can_read(0.01)) {

    chomp(my $line = <STDIN>);

    $sockout->send($line)
      if length $line;

  }

  if($socksel->can_read(0.01)) {

    my $buf = '';
    $sock->recv($buf, 1024);

    print "$buf\n";
  }

}

