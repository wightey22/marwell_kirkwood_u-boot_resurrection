#!/usr/bin/perl
##
## Samba: WINS registration/refresh/release script for wins hook
## Copyright (c) 2008-2011 SATOH Fumiyasu @ OSS Technology, Inc.
##               <http://www.osstech.co.jp/>
## Copyright (c) 2007 oota toshiya
##
## License: GNU General Public License version 2 or later
## Date: 2011-12-01, since 2007-07-05
##

## TODO: Receive a reply packet and check it.
## TODO: Send and receive packets concurrency if multiple WINS server
##       peers are specified.
## TODO: New "exclude names" parameter.
##       ex1: exclude names = FOO BAR BAZ#20
##       ex2: exclude names = DOM#1c/192.168.0.0/24(172.16.0.1)

my $VERSION = 0.23;

## ======================================================================

use strict;
use warnings;
use English qw(-no_match_vars);
use Errno;
use Sys::Syslog;
use File::stat;
use DirHandle;
use IO::File;
use IO::Socket;

use constant true => 1;
use constant false => undef;

use constant NMB_NAME_MAX =>			15;
use constant NMB_PORT =>			137;

use constant NMB_NAME_TYPE_WORKSTATION =>	0x00;
use constant NMB_NAME_TYPE_MESSENGER =>		0x03;
use constant NMB_NAME_TYPE_DMB =>		0x1B;
use constant NMB_NAME_TYPE_DC =>		0x1C;
use constant NMB_NAME_TYPE_BROWSER =>		0x1E;
use constant NMB_NAME_TYPE_NETDDE =>		0x1F;

## NAME_TRN_ID: Transaction ID: 16 bit
use constant NMB_NAME_TRN_ID_MAX =>		0xFFFF;

## OPCODE: Packet type: 5 bit
use constant NMB_OPCODE_MASK =>			0x1F << 11;
use constant NMB_OPCODE_QUERY =>		0x00 << 11;
use constant NMB_OPCODE_REGISTRATION =>		0x05 << 11;
use constant NMB_OPCODE_RELEASE =>		0x06 << 11;
use constant NMB_OPCODE_WACK =>			0x07 << 11;
use constant NMB_OPCODE_REFRESH =>		0x08 << 11;
use constant NMB_OPCODE_MULTIHOMEDREGISTRATION=>0x0F << 11;

## RCODE: Result code: 4 bit
use constant NMB_RCODE_MASK =>			0x0F;
## FMT_ERR: Format error
use constant NMB_RCODE_FMT_ERR =>		0x01;
## SRV_ERR: Server failure
use constant NMB_RCODE_SRV_ERR =>		0x02;
## IMP_ERR: Unsupported request error
use constant NMB_RCODE_IMP_ERR =>		0x04;
## RFS_ERR: Refused error
use constant NMB_RCODE_RFS_ERR =>		0x05;
## ACT_ERR: Active error
use constant NMB_RCODE_ACT_ERR =>		0x06;
## CFT_ERR: Name in conflict error
use constant NMB_RCODE_CFT_ERR =>		0x07;

## NM_FLAGS: Flags for operation: 7 bit
use constant NMB_NM_FLAG_MASK =>		0x7F << 4;
use constant NMB_NM_FLAG_BROADCAST =>		0x01 << 4;
use constant NMB_NM_FLAG_RECURSIONAVAILABLE =>	0x08 << 4;
use constant NMB_NM_FLAG_RECURSIONDESIRED =>	0x10 << 4;
use constant NMB_NM_FLAG_AUTHORITATIVEANSWER =>	0x40 << 4;

## QUESTION_TYPE: Type of the request
## NB: General name service resource record
use constant NMB_Q_TYPE_NB => 0x20;
## QUESTION_CLASS: Type of the request
## IN: Internet class
use constant NMB_Q_CLASS_IN => 0x01;

## RR_TYPE: Type of the resource record
## NB: General name service resource record
use constant NMB_RR_TYPE_NB => 0x20;
## RR_CLASS: Type of the resource record
## IN: Internet class
use constant NMB_RR_CLASS_IN => 0x01;

## NB_FLAGS: Field of the RESOURCE RECORD RDATA field for RR_TYPE of "NB"
## G: Group name flag: 1 bit
use constant NMB_NB_FLAG_GROUP_MASK =>		0x01 << 15;
use constant NMB_NB_FLAG_GROUP =>		0x01 << 15;
## ONT: Owner node type: 2 bit
use constant NMB_NB_FLAG_ONT_MASK =>		0x03 << 13;
use constant NMB_NB_FLAG_ONT_BROADCAST =>	0x00 << 13;
use constant NMB_NB_FLAG_ONT_P2P =>		0x01 << 13;
use constant NMB_NB_FLAG_ONT_MIXED =>		0x10 << 13;
use constant NMB_NB_FLAG_ONT_HYBRID =>		0x11 << 13;

## Options
## ======================================================================

my $cmd_name = $0;
$cmd_name =~ s|.*/||;

my $time = time;
my $flag_syslog = true;
my $syslog_facility = $ENV{'WINS_PUSH_SYSLOG_FACILITY'} || 'daemon';

my $conf_file = $ENV{'WINS_PUSH_CONF'} || '/opt/osstech/etc/samba/wins-push.conf';
my $state_dir = '/opt/osstech/var/lib/samba/wins-push';
my @wins_servers = ();
my $min_ttr = 10;
my @exclude_types = (
  NMB_NAME_TYPE_WORKSTATION,
  NMB_NAME_TYPE_MESSENGER,
  NMB_NAME_TYPE_NETDDE,
);

my $cmd_usage = "Usage: $0 OPERATION NAME TYPE TTL [IP ...]

Operation:
  add
  refresh
  delete
Name:
  NetBIOS name
Type:
  NetBIOS name type (2digit hexadecimal number)
TTL:
  Time To Live in seconds
IP:
  IP address(es)
";

## Logging
## ======================================================================

Sys::Syslog::openlog($cmd_name, 'pid', $syslog_facility);

{
  package Console;
  use Symbol;
  sub new {
    my $sym = gensym;
    tie *$sym, $_[0], $_[1];
    bless $sym, $_[0];
    return $sym;
  }
  sub TIEHANDLE { bless {'write' => $_[1]}, $_[0] }
  sub PRINT { return shift->{'write'}->(@_[1..$#_]); }
  sub WRITE { return shift->{'write'}->(@_[1..$#_]); }
}

my $stdout = (-t STDOUT) ? IO::File->new_from_fd(STDOUT->fileno, 'w') : undef;
my $stderr = (-t STDOUT) ? IO::File->new_from_fd(STDERR->fileno, 'w') : undef;
#*STDOUT = Console->new(\&pinfo);
*STDERR = Console->new(\&perr);

sub pinfo {
  chomp(my $m = join('', @_));
  $stderr->print("$0: INFO: $m\n") if ($stderr);
  syslog('info', $m) if ($flag_syslog);
}

sub pwarn {
  chomp(my $m = join('', @_));
  $stderr->print("$0: WARNING: $m\n") if ($stderr);
  syslog('warning', $m) if ($flag_syslog);
}

sub perr {
  chomp(my $m = join('', @_));
  $stderr->print("$0: ERROR: $m\n") if ($stderr);
  syslog('err', $m) if ($flag_syslog);
}

sub pdie {
  perr(@_);
  exit(1);
}

sub abort_eval {
  chomp(my $m = join('', @_));
  die "$m\n";
}

## Command-line options
## ======================================================================

if (@ARGV < 4) {
  print $cmd_usage;
  exit(0);
}
my ($op_name, $nmb_name, $nmb_name_type, $nmb_name_ttl, @nmb_name_ips) = @ARGV;

if (length($nmb_name) > NMB_NAME_MAX) {
  pdie "NetBIOS name too long (>", NMB_NAME_MAX, "): $nmb_name";
}

if ($nmb_name_type !~ /^[0-9A-F]{1,2}$/i) {
  pdie "invalid name type: $nmb_name_type";
}

if ($nmb_name_ttl !~ /^\d+$/) {
  pdie "invalid TTL: $nmb_name_ttl";
}

@nmb_name_ips = grep {$_ ne '255.255.255.255'} @nmb_name_ips;
if (@nmb_name_ips == 0) {
  exit(0);
}

for my $nmb_name_ip (@nmb_name_ips) {
  if (scalar(grep {/^\d+$/ && $_>=0 && $_<=255} split(/\./, $nmb_name_ip)) != 4) {
    pdie "invalid IP address: $nmb_name_ip";
  }
}

$nmb_name = uc($nmb_name);
$nmb_name_type = hex($nmb_name_type);
if (grep {$_ == $nmb_name_type} @exclude_types) {
  exit(0);
}

my $nmb_name_p = sprintf('%s#%02X', $nmb_name, $nmb_name_type);
my $nmb_name_is_group =
  $nmb_name_type == NMB_NAME_TYPE_DC ||
  $nmb_name_type == NMB_NAME_TYPE_BROWSER;

## Configuration file
## ======================================================================

my $conf_fh = IO::File->new($conf_file) ||
  pdie "cannot open configuration file: $conf_file: $OS_ERROR";

while (!$conf_fh->eof) {
  chomp(my $line = $conf_fh->getline);
  next if ($line =~ /^\s*(?:#.*)?$/);

  eval {
    unless ($line =~ /^\s*([^=]+?)\s*=\s*(.*?)\s*$/) {
      abort_eval "invalid format";
    }
    my $param = $1;
    my $value = $2;

    if ($param eq 'wins servers') {
      @wins_servers = split(/\s*,\s*|\s+/, $value);
    }
    elsif ($param eq 'state directory') {
      $state_dir = $value;
    }
    elsif ($param eq 'min time to resend') {
      $min_ttr = $value;
    }
    elsif ($param eq 'syslog') {
      $flag_syslog = ($value =~ /^(yes|true|1)$/) ? true : false;
    }
    elsif ($param eq 'syslog facility') {
      $syslog_facility = $value;
      Sys::Syslog::openlog($cmd_name, 'pid', $syslog_facility);
    }
    else {
      abort_eval "unknown parameter";
    }
  };
  if ($EVAL_ERROR) {
    chomp($EVAL_ERROR);
    pdie "bad configuration file: $conf_file: $EVAL_ERROR (line ",
      $conf_fh->input_line_number, ")";
  }
}

$conf_fh->close;

## ----------------------------------------------------------------------

if (defined(my $e = $ENV{'WINS_PUSH_STATE_DIR'})) {
  $state_dir = $e;
}

if (defined(my $e = $ENV{'WINS_PUSH_WINS_SERVERS'})) {
  @wins_servers = split(/\s*,\s*|\s+/, $e);
}

## Construct a name service packet
## ======================================================================

## Header
## ----------------------------------------------------------------------

## NAME_TRN_ID: Transaction ID
my $nmb_name_trn_id = int(rand(NMB_NAME_TRN_ID_MAX + 1));

## OPCODE: Operation code
my $nmb_opcode = undef;
if ($op_name eq 'add') {
  if ($nmb_name_is_group) {
    $nmb_opcode = NMB_OPCODE_REGISTRATION;
  }
  else {
    $nmb_opcode = NMB_OPCODE_MULTIHOMEDREGISTRATION;
  }
}
elsif ($op_name eq 'refresh') {
  $nmb_opcode = NMB_OPCODE_REFRESH;
}
elsif ($op_name eq 'delete') {
  $nmb_opcode = NMB_OPCODE_RELEASE;
}
else {
  die "unknown operation name: $op_name";
}

## NM_FLAGS: Flags for operation
my $nmb_nm_flags = 0x00;
if ($op_name eq 'add') {
  $nmb_nm_flags |= NMB_NM_FLAG_RECURSIONDESIRED;
}
## RCODE: Result codes of request
my $nmb_rcode = 0x00;

## QDCOUNT: Number of entries in the question section
my $nmb_qd_count = 1;
## ANCOUNT: Number of entries in the answer section
my $nmb_an_count = 0;
## NSCOUNT: Number of resource records in the authority section
my $nmb_ns_count = 0;
## ARCOUNT: Number of resource records in the additional records
my $nmb_ar_count = 1;

## Construct HEADER part
my $nmb_header = join('',
  pack('n' , $nmb_name_trn_id),
  pack('n' , $nmb_opcode | $nmb_nm_flags | $nmb_rcode),
  pack('nnnn' , $nmb_qd_count, $nmb_an_count, $nmb_ns_count, $nmb_ar_count),
);

## Question section
## ----------------------------------------------------------------------

## QUESTION_NAME: NetBIOS name for the request
my $nmb_qd_name = sprintf('%-'.NMB_NAME_MAX.'s', $nmb_name) . chr($nmb_name_type);
## QUESTION_TYPE: Type of the request
my $nmb_qd_type	= NMB_Q_TYPE_NB;
## QUESTION_CLASS: Class of the request
my $nmb_qd_class = NMB_Q_CLASS_IN;

## Cunstruct "compressed" NetBIOS name
## See samba/source/libsmb/nmblib.c:put_nmb_name()
my @nmb_qd_name = split(//, $nmb_qd_name);
my @nmb_qd_name_bin = ();
for my $i (0..NMB_NAME_MAX) {
  $nmb_qd_name_bin[1+2*$i] = chr(ord('A') + ((ord($nmb_qd_name[$i]) >> 4) & 0xF));
  $nmb_qd_name_bin[2+2*$i] = chr(ord('A') + (ord($nmb_qd_name[$i]) & 0xF));
}
$nmb_qd_name_bin[0] = chr(0x20);
$nmb_qd_name_bin[33] = chr(0x00);
my $nmb_qd_name_bin = join('', @nmb_qd_name_bin);

## Construct QUESTION ENTRY part
my $nmb_qd = join('',
  $nmb_qd_name_bin,
  pack('nn', $nmb_qd_type, $nmb_qd_class),
);

## Additional section (resource record)
## ----------------------------------------------------------------------

## RR_NAME: NetBIOS name for the resource record
## Compressed name: Pointer to the name in the question section
my $nmb_ar_name_bin = chr(0xC0) . chr(length($nmb_header));
## RR_TYPE: Type of the resource record
my $nmb_ar_type = NMB_RR_TYPE_NB;
## RR_CLASS: Class of the resource record
my $nmb_ar_class = NMB_RR_CLASS_IN;
## TTL: The Time To Live of the resource record's name
my $nmb_ar_ttl = $nmb_name_ttl;
## RDLENGTH: Number of bytes in the RDATA field
my $nmb_ar_length = 0x0006; ## == length($nmb_nb_flags . $nmb_nb_address)
## NB_FLAGS: Field of the RESOURCE RECORD RDATA field for RR_TYPE of "NB"
my $nmb_nb_flags = NMB_NB_FLAG_ONT_HYBRID;
if ($nmb_name_is_group) {
  $nmb_nb_flags |= NMB_NB_FLAG_GROUP;
}

## Construct ADDITIONAL RESROUCE RECORD part (without NB_ADDRESS)
my $nmb_ar_wo_ip = join('',
  $nmb_ar_name_bin,
  pack('nn', $nmb_ar_type, $nmb_ar_class),
  pack('N', $nmb_ar_ttl),
  pack('n', $nmb_ar_length),
  pack('n', $nmb_nb_flags),
);

## ======================================================================

my $nmb_packet_wo_ip = $nmb_header . $nmb_qd . $nmb_ar_wo_ip;

for my $nmb_name_ip (@nmb_name_ips) {
  if ($state_dir ne '') {
    my $sent_file = "$state_dir/$nmb_name_p.$nmb_name_ip.$op_name";
    if (my $sent = stat($sent_file)) {
      if ($time - $sent->mtime < $min_ttr) {
	next;
      }
    }
    elsif (!$OS_ERROR{ENOENT}) {
      pdie "stat failed: $sent_file: $OS_ERROR";
    }
    my $sent_fh = IO::File->new($sent_file, 'w') ||
      pdie "cannot update status file: $sent_file: $OS_ERROR";
    $sent_fh->close;
  }

  my $nmb_name_ip_bin = pack('C4', split(/\./, $nmb_name_ip));
  my $nmb_packet = $nmb_packet_wo_ip . $nmb_name_ip_bin;

  for my $wins_server (@wins_servers) {
    my $socket = IO::Socket::INET->new(
      'PeerAddr' => $wins_server,
      'PeerPort' => NMB_PORT,
      'Proto' => 'udp',
    ) || pdie "cannot create a UDP socket: $OS_ERROR";

    pinfo sprintf(
      "push to %s: op=%s, name=%s, ttl=%s, ip=%s\n",
      $wins_server, $op_name, $nmb_name_p, $nmb_name_ttl, $nmb_name_ip,
    );

    $socket->print($nmb_packet);
    ## FIXME: Receive reply packet
  }
}

## Purging old status files
## ----------------------------------------------------------------------

if ($state_dir ne '' && rand(100) < 5) {
  my $status_dh = DirHandle->new($state_dir) ||
    pdie "cannot open status directory: $state_dir: $OS_ERROR";

  while (defined(my $sent_basename = $status_dh->read)) {
    my $sent_file = "$status_dh/$sent_basename";
    next unless (-f $sent_file);

    if (my $sent = stat($sent_file)) {
      if ($time - $sent->mtime > 86400) { ## == 24H * 60M * 60 S
	unlink($sent_file);
      }
    }
    elsif (!$OS_ERROR{ENOENT}) {
      pdie "stat failed: $sent_file: $OS_ERROR";
    }
  }
}

## ======================================================================

exit(0);

