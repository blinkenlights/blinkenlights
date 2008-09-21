package de.blinkenlights.bmix.statistics.gui;

import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectInputStream;
import java.net.InetAddress;
import java.net.Socket;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.prefs.BackingStoreException;
import java.util.prefs.Preferences;

import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;

import de.blinkenlights.bmix.statistics.FrameStatistics;
import de.blinkenlights.bmix.statistics.StatServer;

public class StatsClient implements Runnable {

    private static final Logger logger = Logger.getLogger(StatsClient.class.getName());
    
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

	private void buildGUI() {
	    final JFrame f = new JFrame("BMix statistics: " + serverAddr.getHostAddress());
	    f.add(statsComponent);
	    f.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
	    final Preferences prefs = Preferences.userNodeForPackage(StatsClient.class);
	    f.addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent e) {
                prefs.putInt("frameX", f.getX());
                prefs.putInt("frameY", f.getY());
                prefs.putInt("frameWidth", f.getWidth());
                prefs.putInt("frameHeight", f.getHeight());
                try {
                    prefs.flush();
                } catch (BackingStoreException e1) {
                    logger.log(Level.WARNING, "Prefs flush failed", e1);
                }
                System.exit(0);
            }
	    });
        int x = prefs.getInt("frameX", 20);
        int y = prefs.getInt("frameY", 20);
        int width = prefs.getInt("frameWidth", 800);
        int height = prefs.getInt("frameHeight", 600);
        f.setBounds(x, y, width, height);
	    f.setVisible(true);
    }
	
	public void run() {
	    if (SwingUtilities.isEventDispatchThread()) {
	        throw new RuntimeException("Don't call this on the EDT");
	    }
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
			final StatsClient statsClient = new StatsClient(InetAddress.getByName(bmixHost));
			SwingUtilities.invokeLater(new Runnable() {
                public void run() {
                    statsClient.buildGUI();
                }
            });
			statsClient.run();
		} catch (Exception ex) {
			ex.printStackTrace();
			JOptionPane.showMessageDialog(null, "Startup failed: " + ex);
			// TODO error dialog utility method
		}
	}
}
