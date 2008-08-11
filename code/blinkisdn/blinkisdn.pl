#!/usr/bin/perl -w

use strict;
use IO::Socket;
use IO::Select;
use IO::File;
use Time::HiRes qw(time sleep);
use Capi;


#config

my $localaddr = ':1234';
my $alawdir = '/home/arcade/alaw';
my $logfile = '/home/arcade/blinkisdn.log';
my $updatetime = 1;
my $heartbeattime = 65;
my $setuptime = 4;
my $numchannels = 4;
my $intlesc = '00';
my $ntlesc = '0';
my $cc = '49';
my $ac = '30';

# end

my $bldata = {};
my $chan2bl = {};
my $chan2call = {};
my $call2chan = {};
my $chan2time = {};

my $logfh = undef;

sub log {
  my $string = shift;
  my @d;

  ($d[5], $d[4], $d[3], $d[0], $d[1], $d[2]) = localtime;
  $d[1]++; $d[2] += 1900;

  my $logstr = sprintf("%02d.%02d.%02d %02d:%02d:%02d %s\n", @d, $string);

  ref($logfh)
    and print $logfh $logstr;

  warn $logstr;
}

sub sock2addr {
  my $sock = shift;
  $sock->peerhost . ':' . $sock->peerport;
}

sub bl_sendresp {
  my $addr = shift;
  my $resp = join(':', @_);

  ref($addr)
    and $addr = sock2addr($addr);

  $addr =~ /(.*):(.*)/;

  my $paddr = $1;
  my $pport = $2;

  if(exists $bldata->{$addr}) {
    defined $bldata->{$addr}->{DstPort}
      and $pport = $bldata->{$addr}->{DstPort};
  }

  my $sock = IO::Socket::INET->new(Proto    => 'udp',
				   PeerAddr => $paddr,
				   PeerPort => $pport)
    or return undef;

  $sock->send($resp);
}

sub bl_error {
  my $addr = shift;
  my $chan = shift;

  ref($addr)
    and $addr = sock2addr($addr);

  bl_sendresp($addr, $chan, 'error', @_);

  &log("error: $addr: " . join(':', @_));
}

sub bl_broadcast {
  bl_sendresp($_, @_)
    foreach(keys %$bldata);
}

sub bl_broadcast_state {
  my $conn = shift;
  my $st = shift;

  if($st eq 'SETUP') {

    bl_broadcast($call2chan->{$conn},
		 'setup',
		 $conn->get_calling_party_number || '(unknown)',
		 $conn->get_called_party_number || '(unknown)');

  } elsif($st eq 'CONNECTED') {

    bl_broadcast($call2chan->{$conn},
		 'connected');

  } elsif($st eq 'DISCONNECTED') {
    my $chan = $call2chan->{$conn};
    my $addr = $chan2bl->{$chan};

    bl_broadcast($chan,
		 'onhook');

    delete $call2chan->{$conn};
    undef $chan2call->{$chan};
    defined $addr
      and exists $bldata->{$addr}
	and delete $bldata->{$addr}->{Channels}->{$chan};
    delete $chan2bl->{$chan};
    delete $chan2time->{$chan};

    &log("call on virtual channel $chan was ended");
  }
}

print "BlinkenISDN 2 v1.0.1\n";

$logfh = IO::File->new(">>$logfile")
  or die "can't open logfile '$logfile'!\n";
$logfh->autoflush;

my $capi = Capi->new($intlesc, $ntlesc, $cc, $ac)
  or die "can't initialize capi!\n";

my $sock = IO::Socket::INET->new(Proto     => 'udp',
				 LocalAddr => $localaddr)
  or die "can't open socket at $localaddr!\n";

my $sel = IO::Select->new($sock)
  or die "can't select on socket!\n";

($chan2call->{$_} = undef)
  foreach(1 .. $numchannels);

my $nextupdate = 0;
my $shutdown = undef;

$SIG{INT} =
  sub {
    $_ && $_->disco
      foreach(values %$chan2call);
    $shutdown = 1;
    &log("exiting...");
  };

&log("startup successful - listening on $localaddr");

until($shutdown) {

  if($sel->can_read(0)) {
    my $buf = '';

    $sock->recv($buf, 1024);

    my $addr = sock2addr($sock);

    $buf =~ s/^(\d+):(\w+)//
      or bl_error($addr, 0, 'malformed command'),
	next;

    my $chan = $1;
    my $cmd = "\L$2";
    my $conn = undef;

    unless($cmd eq 'register') {

      exists($bldata->{$addr})
	or bl_error($addr, 0, 'you are not registered'),
	  next;

      unless($cmd eq 'heartbeat') {

	defined($conn = $chan2call->{$chan})
	  or bl_error($addr, $chan, 'no call on this channel'),
	    next;

	!$chan2bl->{$chan} || $bldata->{$addr}->{Channels}->{$chan}
	  or bl_error($addr, $chan, 'not your call'),
	    next;
      }
    }

    if($cmd eq 'register') {

      scalar(keys %$bldata)
	or $capi->listen;

      my $dstport = undef;
      $buf =~ s/^:(\d+)//
	and $dstport = $1;

      &log("client from $addr registered");

      $bldata->{$addr} = { Heartbeat => time + $heartbeattime,
			   DstPort   => $dstport,
			   Channels  => {} };

    } elsif($cmd eq 'heartbeat') {

      $bldata->{$addr}->{Heartbeat} = time + $heartbeattime;

      $buf =~ /^:23/
	and $bldata->{$addr}->{Heartbeat} += 600;
      # super heartbeat

    } elsif($cmd eq 'accept') {

      $chan2bl->{$chan} = $addr;
      delete $chan2time->{$chan};

      $bldata->{$addr}->{Channels}->{$chan} = 1;

      $conn->accept;
	
      $conn->set_dtmf_callback(sub {
				 my $conn = shift;
				 my $chan = $call2chan->{$conn};
				 bl_sendresp($chan2bl->{$chan},
					     $chan,
					     'dtmf',
					     shift);
			       });

      &log("call on virtual channel $chan is owned by $addr");

    } elsif($cmd eq 'hangup') {

      my $reason = undef;

      $buf =~ s/^:(\d+)//
	and $reason = $1;

      $conn->disco($1);

    } elsif($cmd eq 'play' ||
	    $cmd eq 'playbackground') {

      $buf =~ s/^:([^:]*)//
	or bl_error($addr, $chan, 'no filename specified'),
	  next;

      my $fn = $1;
      $fn =~ s/\///sg;
      $fn = "$alawdir/$fn";

      -e $fn
	or bl_error($addr, $chan, 'file does not exist'),
	  next;

      $cmd eq 'play' ?
	$conn->play($fn) :
	  $conn->play_background($fn);

    } else {

      bl_error($addr, 0, 'huh?');

    }

  }

  $capi->do_work;

  if(my $conn = $capi->get_incoming_call) {

    my $chan = undef;
    foreach(sort keys(%$chan2call)) {
      $chan2call->{$_}
	or $chan = $_,
	  last;
    }

    $chan
      or $conn->disco,
	next;

    $chan2call->{$chan} = $conn;
    $call2chan->{$conn} = $chan;
    $chan2time->{$chan} = time + $setuptime;

    $conn->set_state_callback(\&bl_broadcast_state);
    bl_broadcast_state($conn, $conn->get_state);

    &log("incoming call from " .
      ($conn->get_calling_party_number || '(unknown)') .
	" to " .
	  $conn->get_called_party_number .
	    " on virtual channel $chan");
  }

  if(time >= $nextupdate) {

    $nextupdate = time + $updatetime;

    foreach(1 .. $numchannels) {
      my $conn;

      defined($conn = $chan2call->{$_}) ?
	bl_broadcast_state($conn, $conn->get_state) :
	  bl_broadcast($_,
		       'onhook');
    }

    my $kill = undef;

    foreach my $addr (keys %$bldata) {

      if(time > $bldata->{$addr}->{Heartbeat}) {

	bl_error($addr, 0, "no heartbeat for more than $heartbeattime secs - goodbye");
	$chan2call->{$_}->disco
	  foreach(keys %{$bldata->{$addr}->{Channels}});
	delete $bldata->{$addr};
	$kill = 1;
      }
    }

    $kill
      and (scalar(keys %$bldata)
	   or $capi->stop_listen);

  }

  foreach(keys %$chan2time) {

    time > $chan2time->{$_}
      and $chan2call->{$_}->disco;

  }

  sleep(0.01);
}
