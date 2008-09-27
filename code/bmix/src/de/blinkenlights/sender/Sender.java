/*
 * Created on Sep 25, 2008
 */
package de.blinkenlights.sender;

import java.awt.BorderLayout;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;
import java.awt.dnd.DnDConstants;
import java.awt.dnd.DropTarget;
import java.awt.dnd.DropTargetDragEvent;
import java.awt.dnd.DropTargetDropEvent;
import java.awt.dnd.DropTargetEvent;
import java.awt.dnd.DropTargetListener;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.net.InetAddress;
import java.util.List;

import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.JToolBar;
import javax.swing.SwingUtilities;

import de.blinkenlights.bmix.main.BMovieException;
import de.blinkenlights.bmix.main.BMovieSender;

/**
 * This is the main class for the "Stereoscope Player" app. It is a GUI wrapper
 * for the BMovieSender.
 */
public class Sender {

    private static final boolean MAC_OS_X = (System.getProperty("os.name").toLowerCase().startsWith("mac os x")); //$NON-NLS-1$ //$NON-NLS-2$
    private final JLabel statusLabel;
    private final JFrame frame;
    private BMovieSender movieSender;
    private JTextField sendToHost;
    private File currentFile;

    public Sender() {
        frame = new JFrame("Blinkensender");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        
        JMenuBar menuBar = new JMenuBar();
        frame.setJMenuBar(menuBar);
        
        JMenu m = new JMenu("File");
        menuBar.add(m);
        
        m.add(new JMenuItem(new OpenFileAction(frame, this)));
        
        statusLabel = new JLabel("Drag BML file here", JLabel.CENTER);
        frame.add(statusLabel);

        JToolBar configPanel = new JToolBar();
        configPanel.setFloatable(false);
        configPanel.add(new JLabel("Send to: "));
        configPanel.add(sendToHost = new JTextField("localhost"));
        sendToHost.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                sendMovie(currentFile);
            }
        });
        frame.add(configPanel, BorderLayout.SOUTH);
        
        frame.setSize(500, 200);
        
        frame.setDropTarget(new DropTarget(frame, new FileDropListener()));
        frame.setVisible(true);

    }
    
    public void sendMovie(File file) {
        currentFile = file;
        if (currentFile == null) {
            return;
        }
        statusLabel.setText("Sending " + file.getName() + " to "+sendToHost.getText()+":2323");
        try {
            if (movieSender != null) {
                movieSender.stopSending();
            }
            movieSender = new BMovieSender(file.getAbsolutePath(), sendToHost.getText(), 2323, true);
            movieSender.start();
        } catch (BMovieException e) {
            JOptionPane.showMessageDialog(frame, "Couldn't send movie:\n" + e);
        }
    }

    public class FileDropListener implements DropTargetListener {

        public void dragEnter(DropTargetDragEvent dtde) {
            dtde.acceptDrag(DnDConstants.ACTION_COPY);
        }

        public void dragExit(DropTargetEvent dte) {
            // don't care
        }

        public void dragOver(DropTargetDragEvent dtde) {
            dtde.acceptDrag(DnDConstants.ACTION_COPY);
        }

        public void drop(DropTargetDropEvent dtde) {
            try {
                System.out.println("Got drop event: " + dtde);
                Transferable t = dtde.getTransferable();
                List flavorList = dtde.getCurrentDataFlavorsAsList();
                if (t.isDataFlavorSupported(DataFlavor.javaFileListFlavor)) {
                    dtde.acceptDrop(DnDConstants.ACTION_COPY);
                    List fileList = (List) t.getTransferData(DataFlavor.javaFileListFlavor);
                    if (fileList.size() > 0) {
                        sendMovie((File) fileList.get(0));
                    }
                    System.out.println(fileList);
                } else {
                    dtde.rejectDrop();
                    System.out.println("No supported flavours in drop event: " + flavorList);
                }
                dtde.dropComplete(true);
            } catch (Exception ex) {
                JOptionPane.showMessageDialog(null, "Drop failed: " + ex.getMessage());
            }
        }

        public void dropActionChanged(DropTargetDragEvent dtde) {
            // don't care
        }
    }
    
    public static void main(String[] args) {
        System.setProperty("apple.laf.useScreenMenuBar", "true");
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                new Sender();
            }
        });
    }
}
