use 5.006;
use strict;

package Capi;

use warnings;
use Carp;
use Time::HiRes qw(time usleep);

require Exporter;
require DynaLoader;

our @ISA = qw(Exporter DynaLoader);

our $VERSION = '0.03';

bootstrap Capi $VERSION;

sub CAPI_ALERT () {0x01}
sub CAPI_CONNECT () {0x02}
sub CAPI_CONNECT_ACTIVE () {0x03}
sub CAPI_CONNECT_B3_ACTIVE () {0x83}
sub CAPI_CONNECT_B3 () {0x82}
sub CAPI_CONNECT_B3_T90_ACTIVE () {0x88}
sub CAPI_DATA_B3 () {0x86}
sub CAPI_DISCONNECT_B3 () {0x84}
sub CAPI_DISCONNECT () {0x04}
sub CAPI_FACILITY () {0x80}
sub CAPI_INFO () {0x08}
sub CAPI_LISTEN () {0x05}
sub CAPI_MANUFACTURER () {0xff}
sub CAPI_RESET_B3 () {0x87}
sub CAPI_SELECT_B_PROTOCOL () {0x41}
sub CAPI_REQ () {0x80}
sub CAPI_CONF () {0x81}
sub CAPI_IND () {0x82}
sub CAPI_RESP () {0x83}

sub CAPI_CIP_SPEECH () {1}
sub CAPI_CIP_3_1_KHZ_AUDIO () {4}
sub CAPI_CIP_TELEPHONY () {16}
sub CAPI_CIP_GROUP_2_3_FAX () {17}

sub new {
  my $class = shift;
  my ($intlesc, $ntlesc, $cc, $ac) = @_;

  (defined($intlesc) &&
    defined($ntlesc) &&
      defined($cc) &&
	defined($ac)) ||
	  return undef;

  my $self = { Debug       => 0,
	       ConnMap     => {},
	       ApplId      => undef,
	       B3MaxConn   => 30,
	       B3MaxBlks   => 7,
	       B3MaxSize   => 128,
	       MaxMsgs     => 10,
	       MsgNum      => 0,
	       InfoMask    => 16, # Call progression
	       InCalls     => [],
	       ListenMask  => 2 ** CAPI_CIP_SPEECH |
			      2 ** CAPI_CIP_3_1_KHZ_AUDIO |
			      2 ** CAPI_CIP_TELEPHONY |
			      2 ** CAPI_CIP_GROUP_2_3_FAX,
	       Q931MsgType => { 0x01 => 'ALERTING',
				0x02 => 'CALL_PROCEEDING',
				0x03 => 'PROGRESS',
				0x0d => 'SETUP_ACKNOWLEDGE' },
	       IntState    => 'IDLE',
	       DefCause    => { Active => 0 },
	       LastError   => undef,
	       NumIntlEsc  => $intlesc,
               NumNtlEsc   => $ntlesc,
               NumDefCC    => $cc,
	       NumDefAC    => $ac };

  &lib_CAPI20_ISINSTALLED() ||
    return undef;

  ($self->{ApplId} = lib_CAPI20_REGISTER($self->{B3MaxConn},
					 $self->{B3MaxBlks},
					 $self->{B3MaxSize})) ||
    return undef;

  bless $self, $class;

  $self->_capilisten_init($self->{InfoMask}) ||
    return undef;

  $self;
}

sub _err {
  my $s = shift;
  warn("Capi: " . join('', @_) . "\n");
}

sub _debug {
  my $s = shift;
  $s->{Debug} || return;
  $s->_err(@_);
}

sub _get_new_cmsg {
  my $s = shift;

  if(my $cmsg = capi_get_cmsg($s->{ApplId})) {

    my $cmsg = $cmsg->_to_ptr;

    $s->_debug(sprintf('new incoming msg: cmd 0x%02x, subcmd 0x%02x, length 0x%04x',
		       $cmsg->Command,
		       $cmsg->Subcommand,
		       $cmsg->l))
      unless $cmsg->Command == &CAPI_DATA_B3;

    return $cmsg;
  }
}

sub _handle_cmsg {
  my $s = shift;
  my $cmsg = shift;
  my $conn = $s->{ConnMap}->{$cmsg->NCCI & 65535};

  if(ref $conn) {

    $conn->_handle_cmsg($cmsg)

  } else {

    my $cmd = $cmsg->Command;
    my $subcmd = $cmsg->Subcommand;

    if(($cmd == &CAPI_CONNECT) &&
       ($subcmd == &CAPI_IND)) {

      my $inconn = Capi::Connection->new($s, $cmsg);

      if($s->{IntState} ne 'LISTEN' && $s->{DefCause}->{Active}) {
	my $dad = $inconn->get_called_party_number;

	$inconn->disco($s->{DefCause}->{$dad});

      } else {

	push @{$s->{InCalls}}, $inconn;

      }

    } else {

      $s->_err("unsupported command $cmd ($subcmd)!");

    }
  }
}

sub _find_cmsg {
  my $s = shift;
  my $cmsg = shift;
  my $cmd = shift;
  my $subcmd = shift;
  my $msgnum = shift;
  my $cid = shift;

  if(($cmsg->Command == $cmd) &&
     ($cmsg->Subcommand == $subcmd)) {

    defined($msgnum) &&
      (($cmsg->Messagenumber == $msgnum) ||
       return undef);

    if(defined $cid) {
      my $ncci = $cmsg->NCCI;
      ($cid < (2 **  8)) && ($ncci &= 255);
      ($cid < (2 ** 16)) && ($ncci &= 65535);
      ($ncci == $cid) ||
	return undef;
    }

    return $cmsg;
  }

  return undef;
}

sub _wait_cmsg {
  my $s = shift;
  my $cmsg;
  my $found = 0;

  do {

    until($cmsg = $s->_get_new_cmsg) {
      usleep(0.1);
    }

    ($found = $s->_find_cmsg($cmsg, @_)) ||
      $s->_handle_cmsg($cmsg);

  } until($found);

  $cmsg;
}

sub _get_free_controller { 1; }

sub _get_new_msgnum {
  my $s = shift;

  (++$s->{MsgNum} >= (2 ** 16)) &&
    ($s->{MsgNum} = 0);

  $s->{MsgNum};
}

sub _get_new_header {
  my $s = shift;
  my $cmsg = shift;

  ($s->{ApplId}, $cmsg ? $cmsg->Messagenumber : $s->_get_new_msgnum);
}

sub _set_conn_mapping {
  my $s = shift;
  my $val = shift;
  my $conn = shift;

  $s->{ConnMap}->{$val & 65535} = $conn;
}

sub _del_conn_mapping {
  my $s = shift;
  my $val = shift;

  delete $s->{ConnMap}->{$val & 65535};
}

sub _num_to_intl {
  my $s = shift;
  my $num = shift;
  my $ton = shift || 0x00;

  $ton &= 0x70;

  if($ton == 0x00) {

    unless($num =~ /^\+/) {

      ($num =~ /^\d+$/) ||
	return undef;

      ($num =~ s/^\Q$s->{NumIntlEsc}/\+/) ||
	($num =~ s/^\Q$s->{NumNtlEsc}/\+\Q$s->{NumDefCC}/) ||
	  ($num = "+$num");

    }

  } elsif($ton == 0x10) {
    $num = "+$num";

  } elsif($ton == 0x20) {
    $num = "+$s->{NumDefCC}$num";

  } elsif($ton == 0x40) {
    $num = "+$s->{NumDefCC}$s->{NumDefAC}$num";
  }

  $num;
}

sub _num_to_isdn {
  my $s = shift;
  my $num = shift;

  defined($num) ||
    return ();

  ($num =~ s/^\+\Q$s->{NumDefCC}$s->{NumDefAC}//) &&
    return($num, 0x40);

  ($num =~ s/^\+\Q$s->{NumDefCC}//) &&
    return($num, 0x20);

  ($num =~ s/^\+//) &&
    return($num, 0x10);

  ($num, 0x00);
}

sub _called_party_number_ie {
  my $s = shift;
  my $num = shift;
  my $ton = shift;

  if(defined $ton) {

    $ton = ($ton & 0x70) | 0x81;
    return pack('CC', length($num) + 1, $ton) . $num;

  } else {

    $num = $s->_adj_len($num);
    $ton = ord(substr($num, 0, 1, '')) & 0x70;

    return($num, $ton);

  }
}

sub _calling_party_number_ie {
  my $s = shift;
  my $num = shift;
  my $ton = shift;
  my $presentation = shift || 0x00;

  if(defined $ton) {

    $ton = ($ton & 0x70) | 0x01;
    $presentation = ($presentation & 0x60) | 0x80;
    return pack('C3', length($num) + 2, $ton, $presentation) . $num;

  } else {

    $num = $s->_adj_len($num);
    $ton = ord(substr($num, 0, 1, '')) & 0x70;
    $presentation = ord(substr($num, 0, 1, '')) & 0x60;

    return($num, $ton, $presentation);

  }
}

sub _adj_len {
  my $s = shift;
  my $struct = shift;

  (ord(substr($struct, 0, 1, '')) == 255) &&
    substr($struct, 0, 2, '');

  $struct;
}

sub _set_len {
  my $s = shift;
  my $struct = shift;
  defined($struct) || return("\x0");
  my $len = length $struct;

  (($len <= 255) ?
    chr($len) :
      pack('Cv', 255, $len)) .
	$struct;
}

sub _capilisten_init {
  my $s = shift;
  my $infomask = shift || 0;
  my $cipmask = shift || 0;
  my $prof = $s->get_profile;

  for(1 .. $prof->{NumControllers}) {
    LISTEN_REQ($s->_get_new_header,
	       $_,
	       $infomask,
	       $cipmask);

    my $cmsg = $s->_wait_cmsg(&CAPI_LISTEN, &CAPI_CONF);

    my $info = $cmsg->Info;
    unless($info == 0) {
      $s->{LastError} = Capi::Error::CAPI->new($info);
      return undef;
    }
  }

  1;
}

sub _connect {
  my $s = shift;
  my $cip = shift;
  my $dad = shift;
  my $oad = shift;

  my ($num, $ton);
  unless(($num, $ton) = $s->_num_to_isdn($dad)) {
    $s->{LastError} = Capi::Error::Application->new(0xff01);
    return undef;
  }
  $dad = $s->_called_party_number_ie($num, $ton);

  $oad = (($num, $ton) = $s->_num_to_isdn($oad)) ?
    $s->_calling_party_number_ie($num, $ton) :
      "\x0";

  my $msgnum = $s->_get_new_msgnum;

  CONNECT_REQ($s->{ApplId},
	      $msgnum,
	      $s->_get_free_controller,
	      $cip,
	      $dad,
	      $oad,
	      @_);

  my $cmsg = $s->_wait_cmsg(&CAPI_CONNECT, &CAPI_CONF, $msgnum);

  my $info = $cmsg->Info;
  unless(($info == 0) ||
	 ($info == 0x0001) ||
	 ($info == 0x0002)) {
    $s->{LastError} = Capi::Error::CAPI->new($info);
    return undef;
  }

  return Capi::Connection->new($s, $cmsg, [ $cip, $dad, $oad , @_ ]);
}

sub get_profile {
  my $s = shift;
  my $p = { NumControllers => unpack('v', capi20_get_profile()),
	    NumBChannels   => 0 };

  for(1 .. $p->{NumControllers}) {
    (undef, my $numbchan) = unpack('vv', capi20_get_profile($_));
    $p->{NumBChannels} += $numbchan;
  }

  $p;
}

sub do_work {
  my $s = shift;
  my $nummsgs = $s->{MaxMsgs};

  foreach(values %{$s->{ConnMap}}) {
    $_->_do_work;
  }

  while($nummsgs--) {

    my $cmsg;
    ($cmsg = $s->_get_new_cmsg) ?
      $s->_handle_cmsg($cmsg) :
	return 0;

  }

  1;
}

sub get_incoming_call {
  my $s = shift;
  my $dad = shift;
  my $oad = shift;
  my $type = shift;
  my $conn = undef;

  while(my $conn = shift @{$s->{InCalls}}) {

    if(defined $dad) {
      my $c_dad = $conn->get_called_party_number;

      unless(defined($c_dad) && $c_dad =~ /^\Q$dad/) {
	$conn->disco;
	next;
      }
    }

    if(defined $oad) {
      my $c_oad = $conn->get_calling_party_number;

      unless(defined($c_oad) && $c_oad =~ /^\Q$oad/) {
	$conn->disco;
	next;
      }
    }

    return $conn;
  }

  undef;
}

sub wait_for_incoming_call {
  my $s = shift;
  my $was_idle = ($s->{IntState} eq 'IDLE');

  $was_idle &&
    ($s->listen || return undef);

  my $conn;
  until($conn = $s->get_incoming_call(@_)) {
    $s->do_work || usleep(0.1);
  }

  $was_idle &&
    $s->stop_listen;

  $conn;
}

sub call {
  my $s = shift;
  my $dad = shift;
  my $oad = shift;
  my $parms = shift;

  return $s->_connect(CAPI_CIP_SPEECH,
		      $dad,
		      $oad,
		      "\x0",
		      "\x0",
		      1,
		      1,
		      0);
}

sub send_fax {
  my $s = shift;
  my $dad = shift;
  my $oad = shift;
  defined($oad) || ($oad = '');
  my $resolution = shift || 0;
  my $faxsource = shift || return undef;
  my $stationid = shift || $oad;
  my $header = shift;
  defined($header) || ($header = '');

  $resolution =
    ($resolution =~ /(hi|196)/i) ?
      1 : 0;

  $stationid = substr($stationid, 0, 20);

  my $conn = $s->_connect(CAPI_CIP_GROUP_2_3_FAX,
			  $dad,
			  $oad,
			  "\x0",
			  "\x0",
			  4,
			  4,
			  4,
			  "\x0",
			  "\x0",
			  $s->_set_len(pack('vv', $resolution, 0) .
				       $s->_set_len($stationid) .
				       $s->_set_len($header))) ||
    return undef;

  $conn->send($faxsource);

  return $conn;
}

sub keypad {
  my $s = shift;
  my $keypad = shift;
  my $oad = shift;

  return $s->_connect(CAPI_CIP_SPEECH,
		      '',
		      $oad,
		      "\x0",
		      "\x0",
		      1,
		      1,
		      0,
		      "\x0",
		      "\x0",
		      "\x0",
		      "\x0",
		      "\x0",
		      "\x0",
		      "\x0",
		      $s->_set_len($keypad));
}

sub listen {
  my $s = shift;

  unless($s->{DefCause}->{Active}) {
    $s->_capilisten_init($s->{InfoMask}, $s->{ListenMask}) ||
      return undef;
  }

  $s->{IntState} = 'LISTEN';
  1;
}

sub stop_listen {
  my $s = shift;

  unless($s->{DefCause}->{Active}) {
    $s->_capilisten_init($s->{InfoMask}, 0) ||
      return undef;
  }

  $s->{IntState} = 'IDLE';
  1;
}

sub set_default_cause {
  my $s = shift;
  my $dad = shift;
  my $cause = shift;
  my $dc = $s->{DefCause};

  $dad ||
    return undef;

  if($cause) {

    unless($dc->{Active} ||
	   $s->{IntState} eq 'LISTEN') {

      $s->_capilisten_init($s->{InfoMask}, $s->{ListenMask}) ||
	return undef;

    }

    $dc->{$dad} = $cause;
    $dc->{Active} = 1;

  } else {

    delete $dc->{$dad};

    if(keys(%{$dc}) <= 1) {

      $dc->{Active} = 0;

      $s->{IntState} eq 'LISTEN' ||
	$s->_capilisten_init($s->{InfoMask}, 0) ||
	  return undef;
    }

  }

  1;
}

sub get_error {
  my $s = shift;
  my $error = $s->{LastError};

  $s->{LastError} = undef;

  $error;
}

sub DESTROY {
  my $s = shift;

  lib_CAPI20_RELEASE($s->{ApplId});
}


package Capi::Connection;

use Time::HiRes qw(time usleep);

sub new {
  my $class = shift;
  my $capi = shift;
  my $cmsg = shift;

  (ref($capi) && ref($cmsg)) ||
    return undef;

  my $self = { Debug         => 0,
	       State         => undef, # SETUP, CONNECTED, HOLD, DISCONNECTING, DISCONNECTED
	       IntState      => undef, # PRE_SETUP_INCOMING, SETUP_INCOMING, SETUP, SETUP_B3, CONNECTED, HOLD, DISCO_B3_OUTGOING, DISCO
	       Capi          => $capi,
	       LastError     => undef,
	       CID           => undef,
	       PLCI          => undef,
	       ConnectMsgNum => undef,
	       CallData      => { CalledPartyNumber  => undef,
				  CallingPartyNumber => undef,
				  CIPValue           => undef,
				  Cause              => undef,
				  Progress           => undef,
				  FaxInfo            => undef },
	       Timer         => { T304 => { Timeout => 30,
					    Start   => undef } },
	       PlayData      => undef,
	       PlayDataBG    => undef,
	       RecordData    => undef,
	       DataQueue     => {},
	       MaxDataSize   => 128,
	       MaxDataBlocks => 2,
	       StateCB       => undef,
	       ProgressCB    => undef,
	       DtmfCB        => undef };

  bless $self, $class;

  $self->_handle_cmsg($cmsg, @_);

  $self;
}

sub _state {
  my $s = shift;
  my $intstate = shift;
  my $state = shift;

  if($intstate) {
    $s->{IntState} = $intstate;
    $s->_debug("internal state now $intstate");
  }

  if($state &&
     !($s->{State} && $s->{State} eq $state)) {
    $s->{State} = $state;
    $s->{StateCB} &&
      &{$s->{StateCB}}($s, $state);
  }

  wantarray ?
    ($intstate, $state) :
      $intstate;
}

sub _do_work {
  my $s = shift;
  my $t304 = $s->{Timer}->{T304};

  if($t304->{Start} &&
     (time > ($t304->{Start} + $t304->{Timeout}))) {

    $t304->{Start} = undef;
    $s->disco;

    $s->{LastError} = Capi::Error::ISDN->new(0xff03);
  }

  $s->_fill_data_queue;

}

sub _handle_cmsg {
  my $s = shift;
  my $cmsg = shift;
  my $conn_cmsg = shift;
  my $cmd = $cmsg->Command;
  my $subcmd = $cmsg->Subcommand;
  my $capi = $s->{Capi};
  my $cid = $s->{CID};

  if($cmd eq &Capi::CAPI_CONNECT) {

    if($s->{IntState}) {
      $s->_err("wrong state");
      return undef;
    }

    $s->{ConnectMsgNum} = $cmsg->Messagenumber;

    my $cd = $s->{CallData};

    my ($dad_ie, $oad_ie);
    if($conn_cmsg && ref($conn_cmsg)) {

      $cd->{CIPValue} = $conn_cmsg->[0];
      $dad_ie = $conn_cmsg->[1];
      $oad_ie = $conn_cmsg->[2];

    } else {

      $cd->{CIPValue} = $cmsg->CIPValue;
      $dad_ie = $cmsg->CalledPartyNumber;
      $oad_ie = $cmsg->CallingPartyNumber;

    }

    $cd->{CalledPartyNumber} = $capi->_num_to_intl($capi->_called_party_number_ie($dad_ie));
    $cd->{CallingPartyNumber} = $capi->_num_to_intl($capi->_calling_party_number_ie($oad_ie));

    $s->{PLCI} =
      $s->{CID} = $cmsg->NCCI;
    $capi->_set_conn_mapping($s->{CID}, $s);

    if($subcmd eq &Capi::CAPI_IND) {

      $s->_state('PRE_SETUP_INCOMING', 'SETUP');

    } elsif($subcmd eq &Capi::CAPI_CONF) {

      $s->_state('SETUP', 'SETUP');

    }

  } elsif($cmd eq &Capi::CAPI_CONNECT_ACTIVE) {

    ($s->{IntState} eq 'DISCO') &&
      return undef;

    Capi::CONNECT_ACTIVE_RESP($capi->_get_new_header($cmsg),
			      $cid);

    ($s->{IntState} eq 'SETUP') &&
      Capi::CONNECT_B3_REQ($capi->_get_new_header,
			   $cid);

    $s->{Timer}->{T304}->{Start} = undef;

  } elsif($cmd eq &Capi::CAPI_CONNECT_B3) {

    ($s->{IntState} eq 'DISCO') &&
      return undef;

    my $err = 0;

    if($subcmd eq &Capi::CAPI_CONF) {

      my $info = $cmsg->Info;

      unless($info == 0) {

	$s->{LastError} = Capi::Error::CAPI->new($info);
	$s->disco;

	$err = 1;

      }

    } elsif($subcmd eq &Capi::CAPI_IND) {

      Capi::CONNECT_B3_RESP($capi->_get_new_header($cmsg),
			    $cmsg->NCCI,
			    0);

    }

    unless($err) {

      $s->{CID} =
	$cid = $cmsg->NCCI;

      $s->_state('SETUP_B3');

    }

  } elsif($cmd eq &Capi::CAPI_CONNECT_B3_ACTIVE) {

    (($s->{IntState} eq 'DISCO_B3_OUTGOING') ||
     ($s->{IntState} eq 'DISCO')) &&
       return undef;

    Capi::CONNECT_B3_ACTIVE_RESP($capi->_get_new_header($cmsg),
				 $cid);

    $s->_state('CONNECTED', 'CONNECTED');
    $s->_fill_data_queue;

    Capi::FACILITY_REQ($capi->_get_new_header,
		       $cid,
		       1,
		       pack('Cv3CC', 8, 1, 0, 0, 0, 0));

  } elsif($cmd eq &Capi::CAPI_DISCONNECT_B3) {

    if($subcmd eq &Capi::CAPI_IND) {

      Capi::DISCONNECT_B3_RESP($capi->_get_new_header($cmsg),
			       $cid);

      my $reason = $cmsg->Reason_B3;
      ($reason == 0) ||
	($s->{LastError} = Capi::Error::B3Protocol->new($reason));

      $s->{DataQueue} = undef;

      if($s->{CallData}->{CIPValue} ==
	 &Capi::CAPI_CIP_GROUP_2_3_FAX) {

	my ($rate, $res, $format, $pages, $rid) =
	  unpack('vvvvC/a', $capi->_adj_len($cmsg->NCPI));

	$s->{CallData}->{FaxInfo} =
	  { Rate       => $rate,
	    Resolution => $res,
	    Format     => $format,
	    Pages      => $pages,
	    RemoteID   => $rid };

      }

      unless($s->{IntState} eq 'HOLD') {

	# always request a disconnect because it is not safe to assume that
	# after b3 disconnect the isdn connection will also be terminated
	Capi::DISCONNECT_REQ($capi->_get_new_header,
			     $s->{PLCI});

	$s->_state('DISCO', 'DISCONNECTING');

      }
    }

  } elsif($cmd eq &Capi::CAPI_DISCONNECT) {

    if($subcmd eq &Capi::CAPI_IND) {

      Capi::DISCONNECT_RESP($capi->_get_new_header($cmsg),
			    $s->{PLCI});

      my $reason = $cmsg->Reason;

      ($reason == 0x3400) ||
	($reason == 0x3480 + 16) ||
	  ($reason == 0x3480 + 31) ||
	    ($s->{LastError} = Capi::Error::ISDN->new($reason));

      ($reason & 0x3480) &&
	($s->{CallData}->{Cause} = $reason & 0x7f);

      $s->_state('DISCO', 'DISCONNECTED');

      $s->{PlayData} = undef;
      $s->{PlayDataBG} = undef;
      $s->{RecordData} = undef;
      $s->{Timer}->{T304}->{Start} = undef;

      $capi->_del_conn_mapping($cid, $s);

    }

  } elsif($cmd eq &Capi::CAPI_DATA_B3) {

    if($subcmd eq &Capi::CAPI_CONF) {

      delete $s->{DataQueue}->{$cmsg->DataHandle};

      $s->_fill_data_queue;

      my $info = $cmsg->Info;
      ($info == 0) ||
	($s->{LastError} = Capi::Error::B3Protocol->new($info));

    } elsif($subcmd eq &Capi::CAPI_IND) {

      if($s->{RecordData}) {

	$s->{RecordData}->put_data($cmsg->Data) ||
	  ($s->{LastError} = Capi::Error::Application->new(0xff04));

      }

      Capi::DATA_B3_RESP($capi->_get_new_header($cmsg),
			 $cid,
			 $cmsg->DataHandle);

    }

  } elsif($cmd eq &Capi::CAPI_ALERT) {

    if($subcmd eq &Capi::CAPI_CONF) {

      my $info = $cmsg->Info;
      ($info == 0) ||
	($s->{LastError} = Capi::Error::CAPI->new($info));

    }

  } elsif($cmd eq &Capi::CAPI_FACILITY) {

    if($subcmd eq &Capi::CAPI_CONF) {

      my $info = $cmsg->Info;
      ($info == 0) ||
	($s->{LastError} = Capi::Error::CAPI->new($info));

    } elsif($subcmd eq &Capi::CAPI_IND) {

      my $fs = $cmsg->FacilitySelector;
      my $frp = "\x0";
      my $fip = $capi->_adj_len($cmsg->FacilityIndicationParameter);

      if($fs == 3) {
	$frp = pack('CaC',
		    3,
		    substr($fip, 0, 2),
		    0);
      }

      Capi::FACILITY_RESP($capi->_get_new_header($cmsg),
			  $cid,
			  $fs,
			  $frp);


      if($fs == 1) {

	while(length($_ = substr($fip, 0, 1, ''))) {
	  $s->{DtmfCB} &&
	    &{$s->{DtmfCB}}($s, $_);
	}

      } elsif($fs == 3) {

	my ($function, $sssp) =
	  unpack('vC/a', $fip);

	my $ssreason = unpack('v', $sssp);

	if($ssreason == 0) {

	  if($function == 2) {
	    $s->_state('HOLD', 'HOLD');
	  } elsif($function == 3) {
	    $s->_state('SETUP');
	    Capi::CONNECT_B3_REQ($capi->_get_new_header,
				 $cid);
	  }

	} else {
	  $s->{LastError} = Capi::Error::ISDN->new($ssreason);
	}

      }
    }

  } elsif($cmd eq &Capi::CAPI_INFO) {

    if($subcmd eq &Capi::CAPI_IND) {

      Capi::INFO_RESP($capi->_get_new_header($cmsg),
		      $cid);

      my $in = $cmsg->InfoNumber;

      if($in & (2 ** 15)) {

	my $mtype = $capi->{Q931MsgType}->{$in & 0xff};

	$s->{CallData}->{Progress} = $mtype;
	$s->{ProgressCB} &&
	  &{$s->{ProgressCB}}($s, $mtype);

	if($mtype eq 'SETUP_ACKNOWLEDGE') {

	  $s->{Timer}->{T304}->{Start} = time;

	} elsif(($mtype eq 'CALL_PROCEEDING') ||
		($mtype eq 'ALERTING')) {

	  $s->{Timer}->{T304}->{Start} = undef;

	}

      }
    }

  } else {

    $s->_err("unsupported command $cmd ($subcmd)!");

  }
}

sub _accept {
  my $s = shift;

  ($s->{IntState} eq 'PRE_SETUP_INCOMING') ||
    return undef;

  Capi::CONNECT_RESP($s->{Capi}->{ApplId},
		     $s->{ConnectMsgNum},
		     $s->{CID},
		     0,
		     @_);

  $s->_state('SETUP_INCOMING');
}

sub _suppl_service {
  my $s = shift;
  my $function = shift;
  my $ssparm = shift || "\x0";
  my $capi = $s->{Capi};

  Capi::FACILITY_REQ($capi->_get_new_header,
		     $s->{CID},
		     3,
		     $capi->_set_len(pack('v', $function) .
				     $ssparm));

}

sub _fill_data_queue {
  my $s = shift;
  my $capi = $s->{Capi};
  my $dfield = 'PlayData';
  my $data;

  ($s->{IntState} eq 'CONNECTED') ||
    return;

  unless(defined($data = $s->{$dfield})) {
    $dfield = 'PlayDataBG';
    defined($data = $s->{$dfield}) ||
      return;
  }

  my $maxtries = $s->{MaxDataBlocks};
  while((scalar(keys %{$s->{DataQueue}}) < $s->{MaxDataBlocks}) &&
	!$data->is_eod &&
	$maxtries--) {

    my $msgnum = $capi->_get_new_msgnum;
    my $block = $data->get_data($s->{MaxDataSize});

    if(length $block) {

      Capi::DATA_B3_REQ($capi->{ApplId},
			$msgnum,
			$s->{CID},
			$block,
			length $block,
			$msgnum);

      $s->{DataQueue}->{$msgnum} = $block;

    }

  }

  $data->is_eod &&
    undef($s->{$dfield});

}

sub _play {
  my $s = shift;
  my $dfield = shift;
  my $source = shift;
  my $voice = shift;

  if(defined $source) {

    unless($s->{$dfield} = Capi::Data::Reader->new($source,
						   $voice,
						   $dfield eq 'PlayDataBG')) {
      $s->{LastError} = Capi::Error::Application->new(0xff02);
      return undef;
    }

    $s->_fill_data_queue;

  } else {

    undef $s->{$dfield};

  }

  1;
}

sub _record {
  my $s = shift;
  my $dest = shift;
  my $voice = shift;

  if(defined $dest) {

    unless($s->{RecordData} = Capi::Data::Writer->new($dest,
						      $voice)) {
      $s->{LastError} = Capi::Error::Application->new(0xff04);
      return undef;
    }

  } else {

    undef $s->{RecordData};

  }

  1;
}

sub _err {
  my $s = shift;
  warn sprintf("Connection 0x\%08x: \%s\n", $s->{CID}, join('', @_));
}

sub _debug {
  my $s = shift;
  $s->{Debug} || return;
  $s->_err(@_);
}

sub get_state {
  my $s = shift;
  $s->{State};
}

sub set_state_callback {
  my $s = shift;
  my $cb = shift;

  (!defined($cb) || ref($cb))
    || return undef;

  $s->{StateCB} = $cb;
}

sub set_progress_callback {
  my $s = shift;
  my $cb = shift;

  (!defined($cb) || ref($cb))
    || return undef;

  $s->{ProgressCB} = $cb;
}

sub set_dtmf_callback {
  my $s = shift;
  my $cb = shift;

  (!defined($cb) || ref($cb))
    || return undef;

  $s->{DtmfCB} = $cb;
}

sub alerting {
  my $s = shift;
  my $secs = shift;
  my $capi = $s->{Capi};

  ($s->{IntState} eq 'PRE_SETUP_INCOMING') ||
    return undef;

  Capi::ALERT_REQ($capi->_get_new_header,
		  $s->{CID},
		 "\x0");

  if(defined $secs) {
    my $starttime = time;

    while(((time - $starttime) < $secs) &&
	  ($s->{IntState} eq 'PRE_SETUP_INCOMING')) {
      $s->{Capi}->do_work || usleep(0.1);
    }
  }

}

sub call_proceeding {
  # not possible due to a bug in capi-adk.
  # alerting instead
  return alerting(@_);

  my $s = shift;
  my $capi = $s->{Capi};

  ($s->{IntState} eq 'PRE_SETUP_INCOMING') ||
    return undef;

  Capi::ALERT_REQ($capi->_get_new_header,
		  $s->{CID},
		  "\x0",
		  "\x0",
		  "\x0",
		  "\x0",
		  "\x02\x01\x00");
}

sub accept {
  my $s = shift;

  $s->{CallData}->{CIPValue} = &Capi::CAPI_CIP_SPEECH;

  $s->_accept(1, 1, 0);
}

sub accept_fax {
  my $s = shift;
  my $stationid = shift || $s->{CallData}->{CalledPartyNumber};
  my $capi = $s->{Capi};

  $s->{CallData}->{CIPValue} = &Capi::CAPI_CIP_GROUP_2_3_FAX;

  $stationid = substr($stationid, 0, 20);

  $s->_accept(4,
	      4,
	      4,
	      "\x0",
	      "\x0",
	      $capi->_set_len(pack('vv', 0, 0) .
			      $capi->_set_len($stationid) .
			      "\x0"));
}

sub hold {
  my $s = shift;

  ($s->{IntState} eq 'CONNECTED') ||
    return undef;

  $s->_suppl_service(2);
}

sub retrieve {
  my $s = shift;

  ($s->{IntState} eq 'HOLD') ||
    return undef;

  $s->_suppl_service(3);
}

sub ect {
  my $s = shift;
  my $capi = $s->{Capi};

  ($s->{IntState} eq 'HOLD') ||
    return undef;

  $s->_suppl_service(6, $capi->_set_len(pack('V', $s->{PLCI})));
}

sub disconnect { disco(@_) }

sub disco {
  my $s = shift;
  my $cause = shift;
  my $capi = $s->{Capi};
  my $cid = $s->{CID};

  ($s->{IntState} =~ /DISCO/) &&
    return undef;

  if($s->{IntState} eq 'PRE_SETUP_INCOMING') {

    $cause = $cause ? $cause & 0x7f | 0x3480 : 1;

    Capi::CONNECT_RESP($capi->{ApplId},
		       $s->{ConnectMsgNum},
		       $cid,
		       $cause);

    $s->_state('DISCO');

  } elsif(($s->{IntState} eq 'CONNECTED') ||
	  ($s->{IntState} eq 'SETUP_B3')) {

    Capi::DISCONNECT_B3_REQ($capi->_get_new_header,
			    $cid);

    $s->_state('DISCO_B3_OUTGOING');

  } elsif(($s->{IntState} eq 'SETUP') ||
	  ($s->{IntState} eq 'SETUP_INCOMING') ||
	  ($s->{IntState} eq 'HOLD')) {

    Capi::DISCONNECT_REQ($capi->_get_new_header,
			 $cid);

    $s->_state('DISCO');

  } else {

    return undef;

  }

  $s->_state(undef, 'DISCONNECTING');
}

sub get_called_party_number {
  shift->{CallData}->{CalledPartyNumber};
}

sub get_calling_party_number {
  shift->{CallData}->{CallingPartyNumber};
}

sub get_error {
  my $s = shift;
  my $error = $s->{LastError};

  $s->{LastError} = undef;

  $error;
}

sub get_cause {
  shift->{CallData}->{Cause};
}

sub get_progress {
  shift->{CallData}->{Progress};
}

sub get_fax_info {
  shift->{CallData}->{FaxInfo};
}

sub play {
  my $s = shift;
  my $source = shift;
  my $secs = shift;
  my $cip = $s->{CallData}->{CIPValue};

  defined($cip) ||
    return undef;

  ($s->{IntState} =~ /DISCO/) &&
    return undef;

  $s->_play('PlayData',
	    $source,
	    $cip == &Capi::CAPI_CIP_SPEECH) ||
    return undef;

  if(defined $secs) {
    my $starttime = time;
    my $playdata = $s->{PlayData};

    while(( ($secs == 0) || ((time - $starttime) < $secs) ) &&
	  ( defined($s->{PlayData}) && ($playdata eq $s->{PlayData}) )) {
      $s->{Capi}->do_work || usleep(0.1);
    }
  }

  1;

}

sub play_background {
  my $s = shift;
  my $source = shift;
  my $cip = $s->{CallData}->{CIPValue};

  defined($cip) ||
    return undef;

  ($s->{IntState} =~ /DISCO/) &&
    return undef;

  $s->_play('PlayDataBG',
	    $source,
	    $cip == &Capi::CAPI_CIP_SPEECH);
}

sub record {
  my $s = shift;
  my $dest = shift;
  my $secs = shift;
  my $cip = $s->{CallData}->{CIPValue};

  defined($cip) ||
    return undef;

  ($s->{IntState} =~ /DISCO/) &&
    return undef;

  $s->_record($dest,
	      $cip == &Capi::CAPI_CIP_SPEECH) ||
    return undef;

  if(defined $secs) {
    my $starttime = time;
    my $recdata = $s->{RecordData};

    while(( ($secs == 0) || ((time - $starttime) < $secs) ) &&
	  ( defined($s->{RecordData}) && ($recdata eq $s->{RecordData}) )) {
      $s->{Capi}->do_work || usleep(0.1);
    }
  }

  1;
}

sub is_playing {
  shift->{PlayData} && 1;
}

sub is_recording {
  shift->{RecordData} && 1;
}

sub send { play(@_) }

sub receive { record(@_) }

sub DESTROY { disco(@_) }


package Capi::Data;

my $UpSideDown = [ "\x00", "\x80", "\x40", "\xc0", "\x20", "\xa0", "\x60", "\xe0",
		   "\x10", "\x90", "\x50", "\xd0", "\x30", "\xb0", "\x70", "\xf0",
		   "\x08", "\x88", "\x48", "\xc8", "\x28", "\xa8", "\x68", "\xe8",
		   "\x18", "\x98", "\x58", "\xd8", "\x38", "\xb8", "\x78", "\xf8",
		   "\x04", "\x84", "\x44", "\xc4", "\x24", "\xa4", "\x64", "\xe4",
		   "\x14", "\x94", "\x54", "\xd4", "\x34", "\xb4", "\x74", "\xf4",
		   "\x0c", "\x8c", "\x4c", "\xcc", "\x2c", "\xac", "\x6c", "\xec",
		   "\x1c", "\x9c", "\x5c", "\xdc", "\x3c", "\xbc", "\x7c", "\xfc",
		   "\x02", "\x82", "\x42", "\xc2", "\x22", "\xa2", "\x62", "\xe2",
		   "\x12", "\x92", "\x52", "\xd2", "\x32", "\xb2", "\x72", "\xf2",
		   "\x0a", "\x8a", "\x4a", "\xca", "\x2a", "\xaa", "\x6a", "\xea",
		   "\x1a", "\x9a", "\x5a", "\xda", "\x3a", "\xba", "\x7a", "\xfa",
		   "\x06", "\x86", "\x46", "\xc6", "\x26", "\xa6", "\x66", "\xe6",
		   "\x16", "\x96", "\x56", "\xd6", "\x36", "\xb6", "\x76", "\xf6",
		   "\x0e", "\x8e", "\x4e", "\xce", "\x2e", "\xae", "\x6e", "\xee",
		   "\x1e", "\x9e", "\x5e", "\xde", "\x3e", "\xbe", "\x7e", "\xfe",
		   "\x01", "\x81", "\x41", "\xc1", "\x21", "\xa1", "\x61", "\xe1",
		   "\x11", "\x91", "\x51", "\xd1", "\x31", "\xb1", "\x71", "\xf1",
		   "\x09", "\x89", "\x49", "\xc9", "\x29", "\xa9", "\x69", "\xe9",
		   "\x19", "\x99", "\x59", "\xd9", "\x39", "\xb9", "\x79", "\xf9",
		   "\x05", "\x85", "\x45", "\xc5", "\x25", "\xa5", "\x65", "\xe5",
		   "\x15", "\x95", "\x55", "\xd5", "\x35", "\xb5", "\x75", "\xf5",
		   "\x0d", "\x8d", "\x4d", "\xcd", "\x2d", "\xad", "\x6d", "\xed",
		   "\x1d", "\x9d", "\x5d", "\xdd", "\x3d", "\xbd", "\x7d", "\xfd",
		   "\x03", "\x83", "\x43", "\xc3", "\x23", "\xa3", "\x63", "\xe3",
		   "\x13", "\x93", "\x53", "\xd3", "\x33", "\xb3", "\x73", "\xf3",
		   "\x0b", "\x8b", "\x4b", "\xcb", "\x2b", "\xab", "\x6b", "\xeb",
		   "\x1b", "\x9b", "\x5b", "\xdb", "\x3b", "\xbb", "\x7b", "\xfb",
		   "\x07", "\x87", "\x47", "\xc7", "\x27", "\xa7", "\x67", "\xe7",
		   "\x17", "\x97", "\x57", "\xd7", "\x37", "\xb7", "\x77", "\xf7",
		   "\x0f", "\x8f", "\x4f", "\xcf", "\x2f", "\xaf", "\x6f", "\xef",
		   "\x1f", "\x9f", "\x5f", "\xdf", "\x3f", "\xbf", "\x7f", "\xff" ];

sub UpSideDown { $UpSideDown }


package Capi::Data::Reader;

use IO::File;
use IO::Select;
use Fcntl;
use Errno qw(:POSIX);

sub new {
  my $class = shift;
  my $source = shift;
  my $voice = shift || 0;
  my $loop = shift || 0;

  my $self = { Handle     => undef,
	       Select     => undef,
	       Index      => undef,
	       Reader     => undef,
	       Voice      => $voice,
	       Loop       => $loop,
	       EOD        => undef };

  if(ref($source) eq 'SCALAR') {

    $self->{Handle} = $source;
    $self->{Reader} = 'reader_scalar';
    $self->{Index} = 0;

  } elsif(ref($source) eq 'ARRAY') {

    $self->{Handle} = $source;
    $self->{Reader} = 'reader_array';

  } else { # only IO::Handles below

    unless(ref $source) { # filename

      my $fh = IO::File->new($source, O_NONBLOCK);

      $fh ||
	return undef;

      $self->{Handle} = $fh;

    } elsif((ref($source) eq 'GLOB') &&
	    (ref(*{$source}{IO}) eq 'IO::Handle')) {

      $self->{Handle} = *{$source}{IO};

    } elsif(ref($source) eq 'IO::Handle') {

      $self->{Handle} = $source;

    } else {

      return undef;

    }

    $self->{Reader} = 'reader_filehandle';
    $self->{Select} = IO::Select->new($self->{Handle});

    defined($self->{Handle}->blocking(0)) ||
      return undef;

  }

  bless $self, $class;
}

sub get_data {
  my $s = shift;
  my $len = shift;

  my $reader = $s->{Reader};
  my $data = $s->$reader($len);

  ($s->{Voice}) &&
    ($$data =~ s/(.)/&Capi::Data::UpSideDown->[ord $1]/seg);

  $$data;
}

sub is_eod {
  my $s = shift;
  $s->{EOD};
}

sub reader_filehandle {
  my $s = shift;
  my $len = shift;
  my $fh = $s->{Handle};
  my $data = "";

  if($s->{Select}->can_read(0)) {

    my $num = $fh->sysread($data, $len);

    if(defined $num) {
      ($num == 0) &&
	($s->{Loop} ?
	 sysseek($fh, 0, SEEK_SET) :
	 ($s->{EOD} = 1));
    } elsif($! != EAGAIN) {
      $s->{EOD} = 1;
    }

  }

  \$data;
}

sub reader_scalar {
  my $s = shift;
  my $len = shift;
  my $ref = $s->{Handle};
  my $idx = $s->{Index};

  my $data = substr($$ref, $idx, $len);
  defined($data) ||
    ($data = '');

  $idx += $len;
  if($idx >= length($$ref)) {
    $s->{Loop} ?
      $idx = 0 :
	$s->{EOD} = 1;
  }
  $s->{Index} = $idx;

  \$data;
}

sub reader_array {
  my $s = shift;
  my $len = shift;
  my $ref = $s->{Handle};
  my $data = '';

  until(length($data) || !scalar(@$ref)) {

    my $adata = \$ref->[0];

    if(my $type = ref $$adata) {
      ($type eq 'SCALAR') ||
	next;
      $adata = $$adata;
    }

    (defined($$adata) && length($$adata)) ||
      next;

    $data = substr($$adata, 0, $len, '');

    length($$adata) ||
      shift @$ref;

  }

  \$data;
}


package Capi::Data::Writer;

use IO::File;
use IO::Select;
use Fcntl;
use Errno qw(:POSIX);

sub new {
  my $class = shift;
  my $dest = shift;
  my $voice = shift || 0;

  my $self = { Handle     => undef,
	       Select     => undef,
	       Writer     => undef,
	       Voice      => $voice };

  if(ref($dest) eq 'SCALAR') {

    $self->{Handle} = $dest;
    $self->{Writer} = 'writer_scalar';

  } elsif(ref($dest) eq 'ARRAY') {

    $self->{Handle} = $dest;
    $self->{Writer} = 'writer_array';

  } else { # only IO::Handles below

    unless(ref $dest) { # filename

      my $fh = IO::File->new($dest, O_APPEND | O_CREAT | O_WRONLY | O_NONBLOCK);

      $fh ||
	return undef;

      $self->{Handle} = $fh;

    } elsif((ref($dest) eq 'GLOB') &&
	    (ref(*{$dest}{IO}) eq 'IO::Handle')) {

      $self->{Handle} = *{$dest}{IO};

    } elsif(ref($dest) eq 'IO::Handle') {

      $self->{Handle} = $dest;

    } else {

      return undef;

    }

    $self->{Writer} = 'writer_filehandle';
    $self->{Select} = IO::Select->new($self->{Handle});

    defined($self->{Handle}->blocking(0)) ||
      return undef;

  }

  bless $self, $class;
}

sub put_data {
  my $s = shift;
  my $data = shift;

  length($data) ||
    return 1;

  ($s->{Voice}) &&
    ($data =~ s/(.)/&Capi::Data::UpSideDown->[ord $1]/seg);

  my $writer = $s->{Writer};
  $s->$writer(\$data);
}

sub writer_filehandle {
  my $s = shift;
  my $data = shift;
  my $fh = $s->{Handle};
  local $SIG{PIPE} = 'IGNORE';

  ($s->{Select}->can_write(0)) ||
    return undef;

  $! = 0;
  $fh->syswrite($$data);
  !$!;
}

sub writer_scalar {
  my $s = shift;
  my $data = shift;
  my $ref = $s->{Handle};

  $$ref .= $$data;

  1;
}

sub writer_array {
  my $s = shift;
  my $data = shift;
  my $ref = $s->{Handle};

  push @$ref, $$data;

  1;
}


package Capi::Error;

use overload
  '""'   => 'get_desc',
  '0+'   => 'get_code';

my $CodeMap = { 0 => [ 0, '(unknown error)' ],
				
		0xff01 => [ 1, 'Invalid number format' ],
		0xff02 => [ 0, 'Cannot play from source' ],
		0xff03 => [ 0, 'Timer expired' ],
		0xff04 => [ 0, 'Cannot record to destination' ],

		0x0001 => [ 1, 'NCPI not supported by current protocol, NCPI ignored' ],
		0x0002 => [ 1, 'Flags not supported by current protocol, flags ignored' ],
		0x0003 => [ 0, 'Alert already sent by another application' ],
		0x1001 => [ 0, 'Too many applications' ],
		0x1002 => [ 1, 'Logical block size too small; must be at least 128 bytes' ],
		0x1003 => [ 1, 'Buffer exceeds 64 kbytes' ],
		0x1004 => [ 1, 'Message buffer size too small, must be at least 1024 bytes' ],
		0x1005 => [ 1, 'Max. number of logical connections not supported' ],
		0x1007 => [ 0, 'The message could not be accepted because of an internal busy condition' ],
		0x1008 => [ 0, 'OS resource error' ],
		0x1009 => [ 0, 'COMMON-ISDN-API not installed' ],
		0x100A => [ 1, 'Controller does not support external equipment' ],
		0x100B => [ 1, 'Controller does only support external equipment' ],
		
		0x1101 => [ 1, 'Illegal application number' ],
		0x1102 => [ 1, 'Illegal command or subcommand, or message length less than 12 octets' ],
		0x1103 => [ 0, 'The message could not be accepted because of a queue full condition' ],
		0x1104 => [ 0, 'Queue is empty' ],
		0x1105 => [ 0, 'Queue overflow: a message was lost' ],
		0x1106 => [ 1, 'Unknown notification parameter' ],
		0x1107 => [ 0, 'The message could not be accepted because of an internal busy condition' ],
		0x1108 => [ 0, 'OS resource error' ],
		0x1109 => [ 0, 'COMMON-ISDN-API not installed' ],
		0x110A => [ 1, 'Controller does not support external equipment' ],
		0x110B => [ 1, 'Controller supports only external equipment' ],
		
		0x2001 => [ 1, 'Message not supported in current state' ],
		0x2002 => [ 1, 'Illegal Controller/PLCI/NCCI' ],
		0x2003 => [ 0, 'No PLCI available' ],
		0x2004 => [ 0, 'No NCCI available' ],
		0x2005 => [ 0, 'No Listen resources available' ],
		0x2006 => [ 0, 'No fax resources available (protocol T.30)' ],
		0x2007 => [ 1, 'Illegal message parameter coding' ],
		0x2008 => [ 0, 'No interconnection resources available' ],
		
		0x3001 => [ 1, 'B1 protocol not supported' ],
		0x3002 => [ 1, 'B2 protocol not supported' ],
		0x3003 => [ 1, 'B3 protocol not supported' ],
		0x3004 => [ 1, 'B1 protocol parameter not supported' ],
		0x3005 => [ 1, 'B2 protocol parameter not supported' ],
		0x3006 => [ 1, 'B3 protocol parameter not supported' ],
		0x3007 => [ 1, 'B protocol combination not supported' ],
		0x3008 => [ 1, 'NCPI not supported' ],
		0x3009 => [ 1, 'CIP Value unknown' ],
		0x300A => [ 1, 'Flags not supported (reserved bits)' ],
		0x300B => [ 1, 'Facility not supported' ],
		0x300C => [ 1, 'Data length not supported by current protocol' ],
		0x300D => [ 1, 'Reset procedure not supported by current protocol' ],
		0x300E => [ 1, 'TEI assignment failed / overlapping channel masks' ],
		0x300F => [ 1, 'Unsupported interoperability' ],
		0x3010 => [ 1, 'Request not allowed in this state' ],
		0x3011 => [ 1, 'Facility specific function not supported' ],
		
		0x3301 => [ 0, 'Protocol error, Layer 1 (interrupted line or B channel removed by signaling protocol)' ],
		0x3302 => [ 0, 'Protocol error, Layer 2' ],
		0x3303 => [ 0, 'Protocol error, Layer 3' ],
		0x3304 => [ 1, 'Another application got that call' ],
		0x3305 => [ 0, 'Cleared by Call Control Supervision' ],
		0x3311 => [ 1, 'Connection not successful (remote station is not a G3 fax device)' ],
		0x3312 => [ 0, 'Connection not successful (training error)' ],
		0x3313 => [ 1, 'Disconnected before transfer (remote station does not support transfer mode, such as resolution)' ],
		0x3314 => [ 0, 'Disconnected during transfer (remote abort)' ],
		0x3315 => [ 0, 'Disconnected during transfer (remote procedure error)' ],
		0x3316 => [ 0, 'Disconnected during transfer (local Tx data underflow)' ],
		0x3317 => [ 0, 'Disconnected during transfer (local Rx data overflow)' ],
		0x3318 => [ 0, 'Disconnected during transfer (local abort)' ],
		0x3319 => [ 1, 'Illegal parameter coding' ],
		
		0x3480 + 1 => [ 1, 'Unallocated (unassigned) number' ],
		0x3480 + 2 => [ 1, 'No route to specified transit network' ],
		0x3480 + 3 => [ 1, 'No route to destination' ],
		0x3480 + 6 => [ 1, 'Channel unacceptable' ],
		0x3480 + 7 => [ 0, 'Call awarded and being delivered in an established channel' ],
		0x3480 + 16 => [ 0, 'Normal call clearing' ],
		0x3480 + 17 => [ 0, 'User busy' ],
		0x3480 + 18 => [ 0, 'No user responding' ],
		0x3480 + 19 => [ 0, 'No answer from user (user alerted)' ],
		0x3480 + 21 => [ 0, 'Call rejected' ],
		0x3480 + 22 => [ 1, 'Number changed' ],
		0x3480 + 26 => [ 0, 'Non-selected user clearing' ],
		0x3480 + 27 => [ 0, 'Destination out of order' ],
		0x3480 + 28 => [ 1, 'Invalid number format' ],
		0x3480 + 29 => [ 1, 'Facility rejected' ],
		0x3480 + 30 => [ 0, 'Response to STATUS ENQUIRY' ],
		0x3480 + 31 => [ 0, 'Normal, unspecified' ],
		0x3480 + 34 => [ 0, 'No circuit/channel available' ],
		0x3480 + 38 => [ 0, 'Network out of order' ],
		0x3480 + 41 => [ 0, 'Temporary failure' ],
		0x3480 + 42 => [ 0, 'Switching equipment congestion' ],
		0x3480 + 43 => [ 0, 'Access information discarded' ],
		0x3480 + 44 => [ 0, 'Requested circuit/ channel not available' ],
		0x3480 + 47 => [ 0, 'Resources unavailable, unspecified' ],
		0x3480 + 49 => [ 0, 'Quality of service unavailable' ],
		0x3480 + 50 => [ 1, 'Requested facility not subscribed' ],
		0x3480 + 57 => [ 1, 'Bearer capability not authorized' ],
		0x3480 + 58 => [ 0, 'Bearer capability not presently available' ],
		0x3480 + 63 => [ 0, 'Service or option not available, unspecified' ],
		0x3480 + 65 => [ 1, 'Bearer capability not implemented' ],
		0x3480 + 66 => [ 1, 'Channel type not implemented' ],
		0x3480 + 69 => [ 1, 'Requested facility not implemented' ],
		0x3480 + 70 => [ 1, 'Only restricted digital information bearer capability is available' ],
		0x3480 + 79 => [ 1, 'Service or option not implemented, unspecified' ],
		0x3480 + 81 => [ 0, 'Invalid call reference value' ],
		0x3480 + 82 => [ 1, 'Identified channel does not exist' ],
		0x3480 + 83 => [ 0, 'A suspended call exists, but this call identity does not' ],
		0x3480 + 84 => [ 0, 'Call identity in use' ],
		0x3480 + 85 => [ 0, 'No call suspended' ],
		0x3480 + 86 => [ 0, 'Call having the requested call identity has been cleared' ],
		0x3480 + 88 => [ 1, 'Incompatible destination' ],
		0x3480 + 91 => [ 1, 'Invalid transit network selection' ],
		0x3480 + 95 => [ 1, 'Invalid message, unspecified' ],
		0x3480 + 96 => [ 1, 'Mandatory information element is missing' ],
		0x3480 + 97 => [ 1, 'Message type non-existent or not implemented' ],
		0x3480 + 98 => [ 1, 'Message not compatible with call state or message type non-existent or not implemented' ],
		0x3480 + 99 => [ 1, 'Information element non-existent or not implemented' ],
		0x3480 + 100 => [ 1, 'Invalid information element contents' ],
		0x3480 + 101 => [ 1, 'Message not compatible with call state' ],
		0x3480 + 102 => [ 0, 'Recovery on timer expiry' ],
		0x3480 + 111 => [ 0, 'Protocol error, unspecified' ],
		0x3480 + 127 => [ 0, 'Interworking, unspecified' ] };

sub new {
  my $class = shift;
  my $code = shift || 0;
  my $perm = shift;
  my $desc = shift;

  my $self = { Code        => $code,
	       IsPermanent => undef,
	       Description => undef };

  $self->{IsPermanent} =
    defined($perm) ?
      $perm :
	($CodeMap->{$code}->[0] || 0);

  $class =~ /.*\:\:(.*)$/;
  $self->{Description} = "$1: " .
    (defined($desc) ?
       $desc :
         ($CodeMap->{$code}->[1] || $code));

  bless $self, $class;
}

sub get_code { shift->{Code} }

sub is_permanent { shift->{IsPermanent} }

sub get_desc { shift->{Description} }

package Capi::Error::Application;
@Capi::Error::Application::ISA = qw(Capi::Error);

package Capi::Error::CAPI;
@Capi::Error::CAPI::ISA = qw(Capi::Error);

package Capi::Error::ISDN;
@Capi::Error::ISDN::ISA = qw(Capi::Error);

package Capi::Error::B3Protocol;
@Capi::Error::B3Protocol::ISA = qw(Capi::Error);



1;



__END__

=head1 NAME

Capi - Perl extension for the ISDN CAPI interface

=head1 SYNOPSIS

  *** DOCUMENTATION DOES NOT YET EXIST ***

  use Capi;

  my $voice_mailbox_line = '+493012345678';

  my $capi = Capi->new('00', '0', '49', '30');
  my $conn = $capi->wait_for_incoming_call($voice_mailbox_line);

  $conn->accept;

  $conn->play('/sounds/welcome.alaw');


=head1 DESCRIPTION

The Capi module is a perl interface for the Common isdn API.

=head2 EXPORT

None.

=head1 AUTHOR

Tobias Engel, E<lt>perlcapi@tobias.orgE<gt>

=head1 SEE ALSO

L<perl>.

=cut

