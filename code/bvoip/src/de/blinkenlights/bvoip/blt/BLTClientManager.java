package de.blinkenlights.bvoip.blt;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.SocketTimeoutException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.logging.Logger;

import de.blinkenlights.bvoip.Channel;
import de.blinkenlights.bvoip.ChannelList;
import de.blinkenlights.bvoip.asterisk.AGISession;

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
	private final int clientTimeout = 10 * 1000; // milliseconds
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
			try {
				DatagramPacket p = new DatagramPacket(new byte[1024],1024);
				try {
					socket.receive(p);
					logger.fine("got packet from: "+p.getAddress()+": "+new String(p.getData()));
					ClientIdentifier ci = new ClientIdentifier(p.getAddress(),p.getPort());
					BLTCommand bltc = BLTCommand.parsePacket(new String(p.getData()));
					if (bltc.getCommand() == BLTCommand.CommandType.REGISTER) {
						handleRegister(p, ci, bltc);
					}
					else if (bltc.getCommand() == BLTCommand.CommandType.HEARTBEAT) {
						handleHeartBeat(ci);
					}
				} catch (SocketTimeoutException e) {
					logger.finest("timeout");
				}
			} catch (IOException e) {
				logger.warning("got ioexception processing main loop: "+e.getMessage());
			}
			// send channel status
			if (System.currentTimeMillis() > lastStatusSendTime + statusSendInterval) {
				lastStatusSendTime = System.currentTimeMillis();
				logger.finest("sending channel status");
				sendChannelStatus();
				handleTimeouts();
			}
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

	private void handleHeartBeat(ClientIdentifier ci) {
		if (activeClients.containsKey(ci)) {
			BLTClient client = activeClients.get(ci);
			if (client != null) {
				client.heartBeat();
			}
		} else {
			logger.fine("got heartbeat from unregistered client: "+ci);
		}
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
			for (int i = 0; i < channelList.getNumChannels(); i++) {
				BLTCommand command = null;
				if (channelList.isChannelActive(i)) {
					channelScoreboard.append("C");
					command = new BLTCommand(i,BLTCommand.CommandType.CONNECTED);
				} else {
					channelScoreboard.append(".");
					command = new BLTCommand(i,BLTCommand.CommandType.ONHOOK);
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
		logger.fine("got channel connected event!");
		// send SETUP to all connected clients
		AGISession agiSession = channel.getAgiSession();
		BLTCommand command = new BLTCommand(channel.getChannelNum(), BLTCommand.CommandType.SETUP, agiSession.getCallerId(), agiSession.getDnid());
		
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
