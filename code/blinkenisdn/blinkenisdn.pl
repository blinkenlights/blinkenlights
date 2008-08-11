#!/usr/bin/perl -w

use strict;
use IO::Socket;
use Time::HiRes qw(time sleep);

package BlinkenLights::ISDN::Line;

use IO::File;
use IO::Select;
use Time::HiRes qw(time sleep);

sub new {
  my $class = shift;
  my $dev = shift;
  my $msn = shift;
  my $startaudio = shift;
  my $self = { fh           => undef,
	       dev          => $dev,
	       sel          => undef,
	       msn          => $msn,
	       startaudio   => $startaudio,
	       state        => 'unknown',
	       idle_timeout => 180,
	       idle_timer   => undef,
	       timeout      => 5,
	       answer       => 1,
	       in_init      => 0,
	       in_queue     => '' };

  my $fh = IO::File->new($dev, O_RDWR|O_NONBLOCK) || return undef;
  $fh->autoflush(1);
  system("stty <$dev -parenb -parodd cs8 -hupcl -cstopb cread clocal crtscts ignbrk -brkint ignpar -parmrk -inpck -istrip -inlcr -igncr -icrnl -ixon -ixoff -iuclc -ixany -imaxbel -opost -olcuc -ocrnl -onlcr -onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0 -isig -icanon -iexten -echo -echoe -echok -echonl -noflsh -xcase -tostop -echoprt -echoctl -echoke intr undef quit undef erase undef kill undef eof undef eol undef eol2 undef start undef stop undef susp undef rprnt undef werase undef lnext undef flush undef min 255 time 1 38400") && return undef;
  $self->{fh} = $fh;
  $self->{sel} = IO::Select->new($fh);

  $startaudio =~ s/\x10/\x10\x10/g;

  bless $self, $class;

  $self->init || return undef;
  
  $self;
}

sub fill_queue {
  my $self = shift;
  my $queue = '';
  
  if($self->{sel}->can_read(0)) {
    if(defined sysread($self->{fh}, $queue, 1024)) {
      $queue =~ s/[\r\n]+/\n/g;
      $self->{in_queue} .= $queue;
    } else {
      $self->init;
    }
  }

#  warn "$self queue: \"$self->{in_queue}\"\n";
}

sub getchar {
  my $self = shift;

  $self->fill_queue;
  
  length($self->{in_queue}) ? substr($self->{in_queue}, 0, 1, '') : '';
}

sub getline {
  my $self = shift;

  $self->fill_queue;
 
  $self->{in_queue} =~ s/^\n+//;
  return ($self->{in_queue} =~ s/^(.*?\n)//) ? $1 : '';
}

sub block_getresp {
  my $self = shift;
  my $expect = shift;
  my $resp = '';
  
  while($self->{sel}->can_read($self->{timeout})) {
    while(length(my $line = $self->getline)) {
      $resp .= $line;
      return($resp)
	if $resp =~ /\Q$expect\E$/;
    }
  }
  
  return undef;
}

sub say {
  my $self = shift;
  my $cmd = shift;
  my $fh = $self->{fh};
  
#  warn "$self sending command \"$cmd\"\n";
  $self->init
    unless defined syswrite($fh, $cmd);
}

sub command {
  my $self = shift;
  my $cmd = shift;
  my $resp = shift;
  
  $self->say("$cmd\r\n");
  $self->block_getresp("$resp\n");
}

sub init {
  my $self = shift;
  my $init_ok = 0;

  return(0) if $self->{in_init};
  $self->{in_init} = 1;

  warn "modem init...\n";

  $self->fill_queue;
  $self->{in_queue} = '';

  $self->say("\r\n");
  unless($self->command('atz', 'OK')) {
    $self->say("\r\n\x10\x03\r\n");
    $self->{sel}->can_read(1);
    unless($self->getline =~ /VCON/) {
      sleep(4);
      $self->say('+++');
      $self->block_getresp("OK\n");
    }
    $self->say("\r\n");
    $self->command('atz', 'OK');
  }
  if($self->command('ate0', 'OK') &&
     $self->command('ath', 'OK') &&
     $self->command("at\&e$self->{msn}+fclass=8+vsm=6", 'OK')) {
    $init_ok = 1;
  }    

  $self->{state} = ($init_ok ? 'onhook' : 'unknown');
  $self->{idle_timer} = undef;
  $self->{in_init} = 0;

  return $init_ok;
}

sub accept_calls {
  my $self = shift;
  my $answer = shift;
  
  $self->{answer} = ($answer ? 1 : 0);
}

sub getevent {
  my $self = shift;
  
  if($self->{idle_timer} &&
     (time > ($self->{idle_timer} + $self->{idle_timeout}))) {
    warn "idle timeout ($self->{idle_timeout} seconds) - resetting line\n";
    $self->init;
  }

  until(undef) {

    my $line = $self->getline;
    
    if($self->{state} eq 'onhook') {
      
      return $self->{state} unless length $line;
      
      if($self->{answer} &&
	 $line =~ /RING/) {
	
	$self->say("\r\n");

	if($self->command('ata', 'VCON') &&
	   $self->command('at+vtx', 'CONNECT')) {

	  $self->{idle_timer} = time;
	  system("cat $self->{startaudio} >$self->{dev} \&")
	    if $self->{startaudio};
	  return $self->{state} = 'offhook';

	} else {
	  $self->init;
	  
	}
	
      }
      
    } elsif($self->{state} eq 'offhook') {

      if(length $line) {

	if($line =~ /NO CARRIER/) {

	  $self->{idle_timer} = undef;
	  return $self->{state} = 'onhook';
	}
	
      } else {
	
	if($self->getchar eq "\x10") {
	  
	  $self->{idle_timer} = time;
	  my $tone = $self->getchar;
	  return("key$tone")
	    if $tone =~ /^[0-9A-D\*\#]$/;

	}

	return $self->{state};

      }

    } else {
      
      return $self->{state};
      
    }

  }
  
}

sub DESTROY {
  my $self = shift;
  $self->init;
}


package BlinkenLights::ISDN::Test;

use IO::Select;

sub new {
  my $class = shift;
  my $self = { state => 'onhook',
               sel   => undef };

  $self->{sel} = IO::Select->new(\*STDIN);

  bless $self, $class;
}

sub accept_calls {
  return;
}

sub getevent {
  my $self = shift;
  
  if($self->{sel}->can_read(0)) {
    my $line = <STDIN>;
    my $key = substr($line, 0, 1);
    return($self->{state}) unless length $key;
    if($key eq 'o') {
      $self->{state} = (($self->{state} eq 'onhook') ? 'offhook' : 'onhook');
    } elsif($key =~ /[0-9\*\#A-D]/) {
      return("key$key") if $self->{state} eq 'offhook';
    }
  }
  return $self->{state};
}  


package main;

sub usage {
  "usage: $0 <device1> <device2> <msn> <startmessage-audiofile> <server:port> [<delay>]\n";
}

warn "blinkenisdn 1.0\n\n";

my $port1 = shift || die usage;
my $port2 = shift || die usage;
defined(my $msn = shift) || die usage;
my $audiofile = shift || die usage;
my $server = shift || die usage;
my $delay = shift;

defined($delay) || ($delay = 1);

my $line1 = (($port1 eq 'test') ? BlinkenLights::ISDN::Test->new : BlinkenLights::ISDN::Line->new($port1, $msn, $audiofile))
  || die "can't setup line 1 [port $port1] [msn $msn]\n";
warn "line 1 setup [port: $port1] [msn: $msn]\n";

(my $line2 = BlinkenLights::ISDN::Line->new($port2, $msn, $audiofile)) ?
  warn "line 2 setup [port: $port2] [msn: $msn]\n" :
  warn "can't setup line 2 [port $port2] [msn $msn] - using only one line\n";

my $sock = IO::Socket::INET->new(Proto    => 'udp',
				 PeerAddr => $server)
  || die "can't setup udp connection [server: $server]\n";

warn qq{udp connection setup [server: $server]

- send a HUP to stop accepting new calls
- send another HUP to restart accepting calls
- send an INT to exit after last call has finished
- send another INT to reset lines and exit immediately

starting to send events...
};

my $exit = 0;

my $hupsub;
$hupsub = 
  sub { 
    warn "not accepting any incoming calls\n";
    $line1->accept_calls(0) if $line1;
    $line2->accept_calls(0) if $line2;
    $SIG{HUP} =
      sub {
	warn "accepting incoming calls\n";
	$line1->accept_calls(1) if $line1;
	$line2->accept_calls(1) if $line2;
	$SIG{HUP} = $hupsub;
      };
  };
$SIG{HUP} = $hupsub;
$SIG{INT} =
  sub {
    warn "exiting - not accepting any incoming calls\n";
    $line1->accept_calls(0) if $line1;
    $line2->accept_calls(0) if $line2;
    $exit = 1;
    $SIG{INT} = 
      sub { 
	undef $line1;
	undef $line2;
	exit;
      };
  };
$SIG{CHLD} = 'IGNORE';

my $stat = '';
my $oldstat = '';
my $timeout = time;

until($exit && ($stat eq "\x00\x00")) {
  $stat = '';

  foreach my $line ($line1, $line2) {
    
    unless($line) {
      $stat .= "\x00";
      next;
    }

    my $event = $line->getevent;
    if($event eq 'offhook') {
      $stat .= "\xff";
    } elsif($event =~ /key(\S)/) {
      $stat .= $1;
    } else {
      $stat .= "\x00";
    }

  }
  
  if(($stat ne $oldstat) || 
     (time > ($timeout + $delay))) {
    warn sprintf("stat: %02X %02X\n", unpack('CC', $stat));
    $oldstat = $stat;
    print $sock $stat;
    $timeout = time;
  }

  sleep(0.01);
}

undef $line1;
undef $line2;

exit;
