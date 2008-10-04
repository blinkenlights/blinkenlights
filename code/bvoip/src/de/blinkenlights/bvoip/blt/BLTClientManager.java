package de.blinkenlights.bvoip.blt;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketTimeoutException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;

import de.blinkenlights.bvoip.Channel;
import de.blinkenlights.bvoip.ChannelList;
import de.blinkenlights.bvoip.asterisk.AGISession;
import de.blinkenlights.bvoip.blt.BLTCommand.CommandType;

/**
 * A Blinkenlights Telephony protocol client.
 */
public class BLTClientManager implements Runnable  {
	
	private static final Logger logger = Logger.getLogger(BLTClientManager.class.getName());

	private final Map<ClientIdentifier, BLTClient> activeClients = new HashMap<ClientIdentifier, BLTClient>();
	private final int port;
	private final ChannelList channelList;
	private long lastStatusSendTime = 0;
	private final int statusSendInterval = 1000; // one second, in the spec...  Milliseconds!
	private final int clientTimeout = 60 * 1000; // milliseconds
	private final ChannelEventHandler eventHandler = new ChannelEventHandler();
	
	private DatagramSocket socket;

	public BLTClientManager(int port, ChannelList channelList){
		this.port = port;
		this.channelList = channelList;
	}
	
	public void run() {
		logger.entering("BLTClient", "run");
		socket = null;
		try {
			socket = new DatagramSocket(port);
			socket.setSoTimeout(100);
		} catch (IOException e) {
			throw new RuntimeException("couldn't set up socket: ",e);
		}
			
		for (Channel channel : channelList.getChannels()) {
			channel.addPropertyChangeListener(eventHandler);
		}
		
		while (true) {	
			ClientIdentifier ci = null;
			try {
				DatagramPacket p = new DatagramPacket(new byte[1024],1024);
				try {
					socket.receive(p);
					logger.finest("got packet from: "+p.getAddress()+": "+new String(p.getData()));
					ci = new ClientIdentifier(p.getAddress(),p.getPort());
					BLTCommand bltc = BLTCommand.parsePacket(new String(p.getData()));

					if (bltc.getCommand() == BLTCommand.CommandType.REGISTER) {
						handleRegister(p, ci, bltc);
					} else if (activeClients.containsKey(ci)) {
						BLTClient client = activeClients.get(ci);
						if (bltc.getCommand() == BLTCommand.CommandType.HEARTBEAT) {
							handleHeartBeat(client);
						} else if (bltc.getCommand() == BLTCommand.CommandType.ACCEPT) {
							handleAccept(client, bltc.getChannelNum());
						} else if (bltc.getCommand() == BLTCommand.CommandType.HANGUP) {
							handleHangup(client, bltc.getChannelNum());
						} else if (bltc.getCommand() == BLTCommand.CommandType.PLAYBACKGROUND) {
							handlePlaybackground(client, bltc.getChannelNum(), bltc.getArgs().get(0));
						} else if (bltc.getCommand() == BLTCommand.CommandType.PLAY) {
							handlePlay(client, bltc.getChannelNum(), bltc.getArgs().get(0));
						}
					} else {
						logger.fine("got "+bltc.getCommand()+" command from unregistered client: "+ci);
					}
				} catch (SocketTimeoutException e) {
					logger.finest("timeout");
				}
			} catch (Exception e) {
				logger.log(Level.WARNING, "got exception processing main loop", e);
				if(ci != null) {
					BLTClient client = activeClients.get(ci);
					BLTCommand command = new BLTCommand(0, CommandType.ERROR, "bvoip error: " + 
							e.getClass().getName() + " " + e.getMessage());
					sendToClient(client, command);					
				}
			}
			// send channel status
			if (System.currentTimeMillis() > lastStatusSendTime + statusSendInterval) {
				lastStatusSendTime = System.currentTimeMillis();
				logger.finest("sending channel status");
				sendChannelStatus();
				handleTimeouts();
			}
			sendDigits();
		}	
	}

	/**
	 * Sends all the digits to the clients they are due to.
	 */
	private void sendDigits() {
		for (Channel channel : channelList.getChannels()) {
			if (channel.isConnected()) {
				Character digit = channel.getAgiSession().getDigit();
				if (digit != null) {
					BLTCommand digitCommand = new BLTCommand(channel.getChannelNum(), CommandType.DTMF, String.valueOf(digit));
					sendToClient(channel.getClient(), digitCommand);
				}
			}
		}
	}

	private void handlePlay(BLTClient client, int channelNum, String filename) {
		Channel channel = channelList.getChannels().get(channelNum);

		// is the channel registered to another client?
		if(channel.getClient() == null || channel.getClient() != client) {
			logger.fine("Rejecting PLAYBACKGROUND command because this client doesn't own this call");
			BLTCommand command = new BLTCommand(channelNum, CommandType.ERROR, "not your call");
			sendToClient(client, command);
		} else {
			channel.getAgiSession().play(filename);
		}		
	}

	private void handlePlaybackground(BLTClient client, int channelNum, String mohContext) {
		Channel channel = channelList.getChannels().get(channelNum);

		// is the channel registered to another client?
		if(channel.getClient() == null || channel.getClient() != client) {
			logger.fine("Rejecting PLAYBACKGROUND command because this client doesn't own this call");
			BLTCommand command = new BLTCommand(channelNum, CommandType.ERROR, "not your call");
			sendToClient(client, command);
		} else {
			channel.getAgiSession().playBackground(mohContext);
		}
	}
	
	private void handleAccept(BLTClient client, int channelNum) {
		Channel channel = channelList.getChannels().get(channelNum);

		if (channel.getClient() != null) {
			logger.fine("Rejecting ACCEPT command because channel is already accepted");
			BLTCommand command = new BLTCommand(channelNum, CommandType.ERROR, "not your call");
			sendToClient(client, command);
		} else {
			channel.setClient(client);
			channel.getAgiSession().answer();
		}
	}
	
	private void handleHangup(BLTClient client, int channelNum) {
		Channel channel = channelList.getChannels().get(channelNum);
		
		// is the channel registered to another client?
		if(channel.getClient() == null || channel.getClient() != client) {
			logger.fine("Rejecting HANGUP command because this client doesn't own this call: client: "+client+"  channel owner: "+channel.getClient());
			BLTCommand command = new BLTCommand(channelNum, CommandType.ERROR, "not your call");
			sendToClient(client, command);
		} else {
			logger.fine("BLTClient requested hangup on channel: " + channel.getChannelNum());
			channel.getAgiSession().hangup();
		}
	}

	private void handleTimeouts() {
		for (Iterator<BLTClient> clientIter = activeClients.values().iterator(); clientIter.hasNext();) {
			BLTClient client = clientIter.next();
			if (System.currentTimeMillis() > client.getLastHeartBeatTime() + clientTimeout) {
				logger.info("client "+client+" has timed out, removing it");
				clientIter.remove();
			}
		}
		
	}

	private void handleHeartBeat(BLTClient client) {
		client.heartBeat();
	}

	private void handleRegister(DatagramPacket p, ClientIdentifier ci,
			BLTCommand bltc) {
		logger.fine("register, got command with args: "+bltc.getArgs());
		int destPort = p.getPort();
		if (bltc.getArgs().size() == 1) {
			try {
				destPort = Integer.parseInt(bltc.getArgs().get(0));
			} catch (NumberFormatException e) {
				logger.warning("invalid port number sent with registration packet: "+bltc.getArgs().get(0));
			}
		}
		BLTClient client = new BLTClient(ci, destPort);
		if (!activeClients.containsKey(ci)){
			activeClients.put(ci,client);
			logger.info("client "+client+" registered with destPort "+destPort);
			logger.fine("number of clients: "+activeClients.size());
		} else {
			logger.warning("client "+client+" tried to re-register!");
		}
	}



	private void sendChannelStatus() {
		for (Iterator<BLTClient> clientIter = activeClients.values().iterator(); clientIter.hasNext();) {
			BLTClient client = clientIter.next();
			StringBuffer channelScoreboard = new StringBuffer();
			for (Channel channel : channelList.getChannels()) {
				BLTCommand command = null;
				if (channel.isConnected()) {
					channelScoreboard.append("C");
					command = new BLTCommand(channel.getChannelNum(), BLTCommand.CommandType.CONNECTED);
				} else {
					channelScoreboard.append(".");
					command = new BLTCommand(channel.getChannelNum(), BLTCommand.CommandType.ONHOOK);
				}
				sendToClient(client, command);
			}
			logger.finest("Channel Status: "+channelScoreboard);
			
		}
	}

	private void sendToClient(BLTClient client, BLTCommand command) {
		byte[] networkBytes = command.getNetworkBytes();
		DatagramPacket p = new DatagramPacket(networkBytes, networkBytes.length);
		p.setAddress(client.getAddress());
		p.setPort(client.getDestPort());
		try {
			logger.finest("sending packet to "+p.getAddress()+":"+p.getPort()+": "+new String(p.getData()));
			socket.send(p);
		} catch (IOException e) {
			logger.warning("couldn't send to "+client+": "+e.getMessage());
		}
	}
	
	private void channelConnected(Channel channel) {	
		// send SETUP to all connected clients
		AGISession agiSession = channel.getAgiSession();
		String callerId = agiSession.getCallerId();
		String dnid = agiSession.getDnid();
		logger.fine("got channel connected event! callerID="+callerId+" dnid="+dnid);
		BLTCommand command = new BLTCommand(channel.getChannelNum(), BLTCommand.CommandType.SETUP, callerId, dnid);
		
		for (Iterator<BLTClient> clientIter = activeClients.values().iterator(); clientIter.hasNext();) {
			BLTClient client = clientIter.next();
			sendToClient(client, command);
			logger.finest("Sending command "+command+" to client "+client);
		}
		
	}
	
	private class ChannelEventHandler implements PropertyChangeListener {
		public void propertyChange(PropertyChangeEvent evt) {
			Channel source = (Channel) evt.getSource();
			
			// we got a new call
			if (evt.getPropertyName().equals("agiSession")) {
				if (source.getAgiSession() != null) {
					channelConnected(source);
				} else {
					// hangup
				}
			} 
		}
	}
	
}
