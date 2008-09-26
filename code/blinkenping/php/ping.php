#!/usr/local/bin/php -q
<?php
require_once("class_ICMP.php");

// example callback function; see class_ICMP.php for valid $data values
function timeout_callback($callback_type,$data) {
	echo "Agh!  Request timed out!\n";
}

$icmp = new ICMP();

// example of using callbacks to trap specific events; see class_ICMP.php
// for a list of valid callbacks
$icmp->set_callback("timeout","timeout_callback");

$icmp->display = true;

if ($_SERVER["argc"]<2) {
	echo "Usage: ".$_SERVER["argv"][0]." hostname pingcount\n";
  die;
}
if ($_SERVER["argc"]>1) $hostname = $_SERVER["argv"][1];
if ($_SERVER["argc"]>2) $pingcount = $_SERVER["argv"][2]; else $pingcount = 3;

$pingreceived = $icmp->ping($hostname,$pingcount);

echo "\n";
if ($pingcount==$pingreceived) {
  echo "The server responded to all pings.";
} elseif ($pingreceived>0) {
	echo "The server responded to some pings, but there was intermittent packet loss.";
} else {
	echo "The server did not respond.";
}
echo "\n";
?>

