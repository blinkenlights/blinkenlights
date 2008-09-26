<?
// class_ICMP.php - ICMP Ping class
// Version 1.0.1
// Modifications copyright 2001-2002, Steve Blinch (steve at blitzaffe dot com) 
// http://code.blitzaffe.com
//
// Based on code originally by Javier Szyszlican.
// Original code copyright (C) <2002> Javier Szyszlican <javier@szysz.com>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// HISTORY
// ========================================================================
// Much of the code used by this class was originally included with the
// 'Just For Fun' Network Management System by Javier Szyszlican.
//
// While the ICMP code provided with the JFNMS was largely correct, there
// were dozens of logic errors that prevented it from working at all.
//
// This class was created to correct those errors, add several features,
// and neatly encapsulte the ping code into a reusable class.
//
//

function hex2dec(&$item,$key) {
	$item = hexdec($item);
}

/* Default callback function
 *
 * Handles events when no custom handler has been assigned.
 *
 * Accepts:
 *	string $type - The type of event.  Valid events include:
 *		error: an error ocurred
 *		send: a packet was sent
 *		begin: about to start pinging host
 *		receive: echo response received
 *		timeout: packet was not received within time limit
 *		statistics: ping complete; various statistics
 *	array $data - Various data related to the event; varies
 *			depeding on $type.
 */
function cbk($type,$data) {
	switch($type) {
  	case "error":
    	$msg = "Error: ".$data["during"].": ".$data["error"];
    	break;
  	case "send":
    	$msg = "Sent ".$data["bytes"]." bytes (".$data["packet"].")";
  		break;
  	case "begin":
			$msg = "PING ".$data["hostname"]." (".$data["address"]."): ".$data["bytes"]." data bytes";
  		break;
  	case "receive":
    	$msg = $data["bytes"]." bytes from ".$data["address"].": icmp_seq=".$data["packet"]." ttl=".$data["ttl"]." time=".$data["time"]." ms";
  		break;
  	case "timeout":
    	$msg = "Request timed out";
  		break;
  	case "statistics":
    	$msg = "\n --- ".$data["hostname"]." ping statistics ---\n";
      $msg .= $data["transmitted"]." packets transmitted, ".$data["received"]." packets received, ".$data["loss"]."% packet loss\n";
      $msg .= "round-trip min/avg/max = ".$data["mintime"]."/".$data["avgtime"]."/".$data["maxtime"]." ms";
  }
	if ($data["display"]) echo "$msg\n"; else return $msg;
}

/* ICMP Class
 *
 */
class ICMP {

	/* Constructor
   *
   */
	function ICMP() {
  	$this->display = false;
  	$this->verbose = false;
    $this->callbacks = array(
    	"error"=>"cbk",
      "send"=>"cbk",
      "begin"=>"cbk",
      "receive"=>"cbk",
      "timeout"=>"cbk",
      "statistics"=>"cbk"
    );
  }

	function set_callback($type,$functionname) {
  	$this->callbacks[$type] = $functionname;
  }

	/* Retrieves the current time in microseconds.
   *
   * Returns:
   *	int The current time, in microseconds.
   *
   */
  function getmicrotime() {
    list($usec, $sec) = explode(" ",microtime());
    return ((float)$usec + (float)$sec);
  }

	/* Used internally to repack a variable
   *
   */
	function repack ($var) {
    $var = pack("n",$var);
    $temp = unpack("n",$var);
    $aux = $temp[""];
    return $aux;
	}

	/* Used internally to determine the checksum for a packet
   *
   */
	function checksum($buffer) {
    $cksum = 0;
    $counter = 0;
    //var_dump($buffer);
    $i = 1;
    
    foreach ($buffer as $value) {
			if ($i==0) {
	    	$value1 .=$value;
		    $buff1[] = $value1;
		    $i = 1;
		    $value1=0;
			} else {
		    $value1=$value;
	  	  $i = 0;
			}
    }    
    $buff1[] = $value1;
    
    foreach ($buff1 as $value) {
			$aux = substr($value,2,4).substr($value,0,2);
			$value1 = hexdec($aux);
			$cksum += $value1;
    }

    $aux = $this->repack(($cksum & 0xffff));

    $cksum1 = $this->repack((($cksum >> 16) + $aux));
    $cksum2 = $this->repack(($cksum1 + ($cksum1 >> 16)));
    $ans = ~$cksum2;
    $ans = $this->repack($ans);

    $csum1 = dechex($ans);
    $buffer[2] = substr($csum1,2,4);
    $buffer[3] = substr($csum1,0,2);

    return $buffer;
	}

	/* DEBUG: Dumps the contents of a packet to stdout.
   *
   */
	function view_packet($data){
    $datas = explode(" ",$data);
    $i = 1;
    foreach ($datas as $aux) {

			$val =  hexdec($aux);
			$val2 = chr($val);
			$val3 = decbin($val);
			$val4 = str_pad($val3,8,"0",STR_PAD_LEFT); //bin

			$show1.= $aux; //hex
			$show4.= $val4; //bin
	
			if ((($i%2)==0))  $show1.= " ";
			if ((($i%2)==0))  $show4.= " ";

			if ($i == 8) {
	  	  $show1.= "\n";
		    $show4.= "\n";
		    $i=1;
			} else {
		    $i++;
			}
    }
    echo "\n------------------------------------------\n";
    echo "$show1\n\n$show4";
    //echo "$show1";
    echo "\n------------------------------------------\n";

    flush();

	}

	/* Used internally to call the appropriate callback for an event.
   *
   */
	function callback($type,$data) {
  	if ($this->callbacks["all"]) {
    	$cbk = $this->callbacks["all"];
    } elseif ($this->callbacks[$type]) {
      $cbk = $this->callbacks[$type];
      if ($cbk=="cbk") $data["display"] = $this->display;
    } else {
  		return false;
  	}
  	$cbk($type,$data);
  }

	/* Used internally to call the "error" callback.
   *
   */
	function error($during,$msg) {
  	$this->callback(
    	"error",
      array(
      	"during"=>$during,
        "error"=>$msg
      )
    );
  }

	/* Used internally to send a single ICMP ECHO_ECHO packet to the
   * remote host.
   *
   * Returns:
   *	int The number of bytes sent to the remote host, or false on reror.
   */
	function send_ping() {

  		$this->pktno++;
			$conn = socket_connect($this->sock,$this->ipaddr,0);
      if (!$conn) {
				$this->error("connecting to socket",socket_strerror(socket_last_error()));
        return false;
      }
    
			$this->datas[6]= "05"; // type (internal);
			$this->datas[7]= str_pad(dechex($this->pktno),2,"0",STR_PAD_LEFT); //seqnum
    
    	$datas_aux = $this->checksum($this->datas);

			//view_packet(join(" ",$datas_aux));

			unset($val1);
      foreach ($datas_aux as $aux) {
				$val =  hexdec($aux);
				$val2 = chr($val);
				$val1 .= $val2;
	    }

      $this->times[$this->pktno] = $this->getmicrotime();
	    $aux = socket_write($this->sock,$val1,strlen($val1));
      if ($aux === false) {
      	$this->error("writing to socket",socket_strerror(socket_last_error()));
      } elseif ($this->verbose) {
  			$this->callback(
      		"send",
					array(
	        	"bytes"=>$aux,
	          "packet"=>$this->pktno
          )
        );
    	}
      return $aux;

	}

	/* Pings a remote host.
   *
   * Accepts:
   *	$hostname - The hostname of the host to ping (IP or hostname acceptable).
   *  $maxpackets - The number of packets to send. (optional; defaults to 3).
   *
   * Returns:
   *	mixed The number of packets successfully received from the remote host
   *			or false on error.  0 indicates that the host did not reply at all.
   */
	function ping($hostname,$maxpackets=3) {
  	$this->hostname = $hostname;
  	$this->maxpackets = $maxpackets;
    $this->pktno = 0;

    //       ty cd cksum ident seqnu data ---- 
    $data = "08 00 00 00 08 97 00 00 ".
				    "65 00 00 39 8e d8 3c a1 ". 
	    "e1 05 ".
		  "00 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 14 ". //data
	    "15 16 17 18 19 1a 1b 1c 1d 1e 1f 20 21 22 23 24 ". 
	    "25 26 27 28 29 2a 2b 2c 2d 2e 2f 30 31 32 33 34";
	    //"35 36 37";

		unset($this->sock);
		$this->sock = socket_create(AF_INET,SOCK_RAW,getprotobyname("ICMP"));
    if ($this->sock === false) {
      $this->error("creating socket",socket_strerror(socket_last_error()));
      return false;
    }

		$this->datas = explode(" ",$data);
    $this->ipaddr = gethostbyname($this->hostname);

    $this->callback(
    	"begin",
      array(
    		"hostname" => $this->hostname,
        "address" => $this->ipaddr,
        "bytes" => count($this->datas)
      )
    );

		for ($i = 0; $i < $this->maxpackets; $i++) {
    	$aux = $this->send_ping();
  	  if ($aux > 1) {
				$num = 0;
				$timeout = 0;

				while (($num <= 0) and ($timeout < 100))  {
          $set = array($this->sock);
			    $num = socket_select($set, $s_write = NULL, $s_accept = NULL, 0, 1000);
          if ($num === false) {
            $this->callback("waiting on socket",socket_strerror(socket_last_error()));
          }
					$timeout++;
				}

				if ($num>0) {
			    $aux = socket_read($this->sock,100);

          $rxtime = $this->getmicrotime();

			    $len = strlen($aux)-20;
			    unset($val3);
	   	    for ($o = 1; $o < strlen($aux) ;$o++) {
						$val = $aux[$o];
						$val1 = ord($val);
						$val2 = str_pad(dechex($val1),2,"0",STR_PAD_LEFT);
						$val3 .="$val2 ";
	        }

		    	$data_recv = explode(" ",$val3);
		    	//view_packet($val3);

		    	$src =  array_slice($data_recv,11,4); array_walk($src,"hex2dec");
		    	$dest = array_slice($data_recv,15,4); array_walk($dest,"hex2dec");
			    $type = $data_recv[19];
			    $seq1 = array_slice($data_recv,25,2); array_walk($seq1,"hex2dec");
			    $source = join(".",$src);
			    $destination = join(".",$dest);
			    $seq = join(":",$seq1);
			    $ttl = hexdec($data_recv[7]);

			    $txtime = $this->times[$seq1[1]];
					$rtt_msec = ($rxtime - $txtime) * 1000;
			    $total_rtt[]=$rtt_msec;

          $this->callback(
          	"receive",
            array(
            	"bytes"=>$len,
              "address"=>$source,
              "packet"=>$seq1[1],
              "ttl"=>$ttl,
              "time"=>sprintf("%.2f",$rtt_msec)
            )
          );
				} else {
					$this->callback(
          	"timeout",
            array()
          );
	      }
	    }
		}

	  $this->packetsrecv = count($total_rtt);
		socket_close($this->sock);

    $rtt_min = -1;
    $rtt_max = 0;
    if (is_array($total_rtt)) {
			foreach ($total_rtt as $rtt) {
			 	$rtt_total+=$rtt;
        if ($rtt>$rtt_max) $rtt_max = $rtt;
        if ($rtt<$rtt_min || $rtt_min<0) $rtt_min = $rtt;
	    }
			$rtt_avg = round($rtt_total/$this->packetsrecv,3);
    } else {
			$rtt_total = 0;
      $rtt_avg = 0;
      $pct = 100;
		}

		$this->callback(
    	"statistics",
      array(
    		"hostname"=>$this->hostname,
        "transmitted"=>$this->maxpackets,
        "received"=>$this->packetsrecv,
				"loss"=>(100-(($this->packetsrecv/$this->maxpackets)*100)),
        "avgtime"=>sprintf("%.2f",$rtt_avg),
        "mintime"=>sprintf("%.2f",$rtt_min),
        "maxtime"=>sprintf("%.2f",$rtt_max)
      )
    );

		return $this->packetsrecv;
	}

}

?>
