package de.blinkenlights.bmix.statistics;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectOutputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.logging.Logger;


public class StatServer implements Runnable {
	private static final Logger logger = Logger.getLogger(StatServer.class.getName());

	public static final int STAT_PORT = 4892;
	ServerSocket serverSocket;
	List<Socket> clientSockets; 
	
	
	/**
	 * Creates a status server.
	 * 
	 * @throws IOException on error 
	 */
	public StatServer() throws IOException {
		serverSocket = new ServerSocket(STAT_PORT);
		clientSockets = Collections.synchronizedList(new ArrayList<Socket>());
	}
	
	
	public void run() {
		while(true) {
			try {
				Socket clientSocket = serverSocket.accept();
				logger.info("Stats client accepted from: " + clientSocket.getInetAddress().getHostAddress());
				clientSockets.add(clientSocket);
				logger.info("Stats server has: " + clientSockets.size() + " clients");
			} catch (IOException e) {
				logger.warning(e.getMessage());
			}
		}
	}
	
	
	public void sendToClients(FrameStatistics frameStats) throws IOException {
		// don't waste time if we don't have any clients
		if(clientSockets.size() == 0) {
			return;
		}
		
		ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
		ObjectOutputStream outputStream = new ObjectOutputStream(byteStream);
		outputStream.writeObject(frameStats);
		outputStream.flush();
		outputStream.close();
		final byte[] byteArray = byteStream.toByteArray();
		
		for (Iterator<Socket> iter = clientSockets.iterator(); iter.hasNext();) {
			Socket clientSocket = iter.next();
			try {
				OutputStream clientOut = clientSocket.getOutputStream();
				clientOut.write(byteArray);
				clientOut.flush();
			} catch(IOException e) {
				logger.warning("error sending to client: " + e.getMessage() + " - removing it!");
				iter.remove();
				logger.info("Stats server has: " + clientSockets.size() + " clients");
			}
		}
	}
}
