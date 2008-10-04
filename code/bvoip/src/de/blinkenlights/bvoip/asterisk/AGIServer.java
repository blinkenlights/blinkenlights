package de.blinkenlights.bvoip.asterisk;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Collections;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.logging.Level;
import java.util.logging.Logger;

import de.blinkenlights.bvoip.Channel;
import de.blinkenlights.bvoip.ChannelList;

public class AGIServer implements Runnable {
	private static final Logger logger = Logger.getLogger(AGIServer.class.getName());
	private final int port;
	private final ChannelList channelList;
	
	private Set<String> activeLines = Collections.synchronizedSet(new HashSet<String>());
	private final Map<String, Integer> exclusionGroups;
	private Set<Integer> groupsInUse = new HashSet<Integer>();
		
	/**
	 * Creates a new AGIServer
	 * 
	 * @param port the port number to listen on
	 * @param exclusionGroups 
	 */
	public AGIServer(int port, ChannelList channelList, Map<String, Integer> exclusionGroups) {
		this.port = port;
		this.channelList = channelList;
		this.exclusionGroups = exclusionGroups;
	}
	
	
	/**
	 * Runs the AGIServer.
	 */
	public void run() {
		ServerSocket serverSocket = null;
		try {
			serverSocket = new ServerSocket(port);
		} catch (IOException e) {
			e.printStackTrace();
		}
		
		logger.info("listening for asterisk connections on port: " + port);

		while(true) {
			try {
				Socket client = serverSocket.accept();
				handleCall(client);
				
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}


	/**
	 * Handles a new call.
	 * 
	 * @param client the client socket
	 * @throws IOException if there is a communication error with Asterisk
	 */
	private void handleCall(final Socket client) throws IOException {
		if(client == null) throw new NullPointerException("client socket is null");
		logger.info("call received from asterisk - " + client.getInetAddress().getHostAddress());
		
		Runnable callHandler = new Runnable() {
			public void run() {
				AGISession agiSession = null;
				try {
					BufferedReader in = new BufferedReader(new InputStreamReader(client.getInputStream()));
					PrintWriter out = new PrintWriter(client.getOutputStream(), true);
					Channel channel = null;
					boolean acceptedCall = false;
					try {
						channel = channelList.acquire();
						if (channel != null) {
							logger.info("Trying to accept call on channel " + channel);
							agiSession = new AGISession(in, out);
							String dnid = agiSession.getDnid();
							Integer exclGroup = exclusionGroups.get(dnid); // might be null
							if (groupsInUse.contains(exclGroup)) {
								logger.info("Not accepting call because exclusion group " + exclGroup + " is in use");
								agiSession.hangup();
							} else if (activeLines.contains(dnid)) {
								logger.info("Not accepting call because " + dnid + " is already an active line");
								agiSession.hangup();
							} else {
								logger.info("Accepting call");
								acceptedCall = true;
								if (exclusionGroups.containsKey(dnid)) {
									groupsInUse.add(exclusionGroups.get(dnid));
								}
								activeLines.add(dnid);
								channel.setAgiSession(agiSession);
								agiSession.handleCall();
							}
						} else {
							logger.warning("Refusing call (congestion) because all channels are in use");
							AGISession tempAgiSession = new AGISession(in, out);
							tempAgiSession.hangup();
						}
					} catch (CallEndedException ex) {
						logger.info("Call ended: " + channel);
					} catch (Exception ex) {
						logger.log(Level.WARNING, "Call handling ended abnormally", ex);
					} finally {
						if (acceptedCall && agiSession != null) {
							String dnid = agiSession.getDnid();
							activeLines.remove(dnid);
							if (exclusionGroups.containsKey(dnid)) {
								groupsInUse.remove(exclusionGroups.get(dnid));
							}
						}
						if (channel != null) {
							channel.close();
						}
						client.close();
					}
				} catch (Exception ex) {
					ex.printStackTrace();
				}
			}
		};
		
		new Thread(callHandler).start();
	}
}
