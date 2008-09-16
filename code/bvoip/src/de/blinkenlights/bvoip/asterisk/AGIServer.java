package de.blinkenlights.bvoip.asterisk;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.logging.Level;
import java.util.logging.Logger;

import de.blinkenlights.bvoip.Channel;
import de.blinkenlights.bvoip.ChannelList;

public class AGIServer implements Runnable {
	private static final Logger logger = Logger.getLogger(AGIServer.class.getName());
	private final int port;
	private final ChannelList channelList;
	
	/**
	 * Creates a new AGIServer
	 * 
	 * @param port the port number to listen on
	 */
	public AGIServer(int port, ChannelList channelList) {
		this.port = port;
		this.channelList = channelList;
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
				try {
					BufferedReader in = new BufferedReader(new InputStreamReader(client.getInputStream()));
					PrintWriter out = new PrintWriter(client.getOutputStream(), true);
					Channel channel = null;
					try {
						channel = channelList.acquire();
						if (channel != null) {
							AGISession agiSession = new AGISession(in, out);
							channel.setAgiSession(agiSession);
							agiSession.handleCall();
						} else {
							logger.warning("Ingoring call because all channels are in use");
						}
					} catch (CallEndedException ex) {
						logger.info("Call ended: " + channel);
					} catch (Exception ex) {
						logger.log(Level.WARNING, "Call handling ended abnormally", ex);
					} finally {
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
