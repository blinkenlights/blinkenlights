package de.blinkenlights.bmix.statistics.gui;

import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectInputStream;
import java.net.InetAddress;
import java.net.Socket;
import java.util.prefs.Preferences;

import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;

import de.blinkenlights.bmix.statistics.FrameStatistics;
import de.blinkenlights.bmix.statistics.StatServer;

public class StatsClient implements Runnable {

	private final InputStream in;
	private final StatsComponent statsComponent;
	private final InetAddress serverAddr;

	public StatsClient(InetAddress serverAddr) throws IOException {
		this.serverAddr = serverAddr;
		Socket s = new Socket(serverAddr, StatServer.STAT_PORT);
		in = s.getInputStream();
		s.getOutputStream();
		if (!s.isConnected()) throw new IOException("Not connected to server");
		System.out.println("Connected");
		statsComponent = new StatsComponent();
	}

	public void run() {
		JFrame f = new JFrame("BMix statistics: " + serverAddr.getHostAddress());
		f.add(statsComponent);
		f.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		f.pack();
		f.setVisible(true);
		try {
			for (;;) {
				FrameStatistics stats = (FrameStatistics) new ObjectInputStream(in).readObject();
				statsComponent.update(stats);
			}
		} catch (Exception ex) {
			ex.printStackTrace();
			JOptionPane.showMessageDialog(null, "Failure: " + ex);
			// TODO error dialog utility method
		}
	}
	
	public static void main(String[] args) {
		try {
			Preferences prefs = Preferences.userNodeForPackage(StatsClient.class);
			String bmixHost = prefs.get("bmixHost", "localhost");
			bmixHost = JOptionPane.showInputDialog("Connect to which bmix?", bmixHost);
			prefs.put("bmixHost", bmixHost);
			prefs.flush();
			StatsClient statsClient = new StatsClient(InetAddress.getByName(bmixHost));
			SwingUtilities.invokeLater(statsClient);
		} catch (Exception ex) {
			ex.printStackTrace();
			JOptionPane.showMessageDialog(null, "Startup failed: " + ex);
			// TODO error dialog utility method
		}
	}
}
