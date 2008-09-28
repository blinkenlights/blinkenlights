/*
 * Created on Sep 25, 2008
 */
package de.blinkenlights.sender;


import com.apple.mrj.*;

import java.awt.BorderLayout;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
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
import java.util.List;

import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;

import de.blinkenlights.bmix.main.BMovieException;
import de.blinkenlights.bmix.main.BMovieSender;

/**
 * This is the main class for the "Stereoscope Player" app. It is a GUI wrapper
 * for the BMovieSender.
 */
public class Sender implements MRJOpenDocumentHandler {

    private static final String DEFAULT_STATUS_MESSAGE = "<html><center>Drag BML file here</center>";
    private final JLabel statusLabel;
    private final JFrame frame;
    private BMovieSender movieSender;
    private final JTextField sendToHost;
    private File currentFile;
    private final JButton playButton;
    private final JButton stopButton;
    private final JCheckBox loop;

    public Sender() {
    		MRJApplicationUtils.registerOpenDocumentHandler(this);
    
        frame = new JFrame("Stereoscope Player");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        
        JMenuBar menuBar = new JMenuBar();
        frame.setJMenuBar(menuBar);
        
        JMenu m = new JMenu("File");
        menuBar.add(m);
        
        m.add(new JMenuItem(new OpenFileAction(frame, this)));
        
        statusLabel = new JLabel(DEFAULT_STATUS_MESSAGE, JLabel.CENTER);
        frame.add(statusLabel);

        JPanel configPanel = new JPanel(new GridBagLayout());
        configPanel.setBorder(BorderFactory.createEmptyBorder(0, 7, 4, 15));
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.fill = GridBagConstraints.HORIZONTAL;
        gbc.weightx = 0f;
        configPanel.add(new JLabel("Send to: "), gbc);
        gbc.weightx = 1f;
        configPanel.add(sendToHost = new JTextField("localhost"), gbc);
        sendToHost.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                sendMovie(currentFile);
            }
        });
        
        gbc.weightx = 0f;
        playButton = new JButton("Play");
        playButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (currentFile != null) {
                    sendMovie(currentFile);
                } else {
                    JOptionPane.showMessageDialog(
                            frame,
                            "Please open a movie before attempting to play it!",
                            "Can't play nothing",
                            JOptionPane.WARNING_MESSAGE);
                }
            }
        });
        configPanel.add(playButton, gbc);

        gbc.weightx = 0f;
        stopButton = new JButton("Stop");
        stopButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (movieSender != null) {
                    movieSender.stopSending();
                    statusLabel.setText("<html><center>Stopped sending. Press Play to restart.</center>");
                }
            }
        });
        configPanel.add(stopButton, gbc);

        loop = new JCheckBox("Loop", true);
        loop.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (movieSender != null) {
                    movieSender.setLooping(loop.isSelected());
                }
            }
        });
        configPanel.add(loop, gbc);
        
        frame.add(configPanel, BorderLayout.SOUTH);
        
        frame.setSize(500, 200);
        
        frame.setDropTarget(new DropTarget(frame, new FileDropListener()));
        frame.setVisible(true);

    }

		public void handleOpenFile(File fileName) {
			this.sendMovie(fileName);
		}
		
    public void sendMovie(File file) {
        currentFile = file;
        if (currentFile == null) {
            return;
        }
        statusLabel.setText("<html><center>" +
        		            "Sending " + file.getName() + " to "+sendToHost.getText()+":2323<br>" +
        		            "<br>" +
                            "Please open Stereoscope Simulator to view the output" +
                            "</center>");
        try {
            if (movieSender != null) {
                movieSender.stopSending();
            }
            movieSender = new BMovieSender(file.getAbsolutePath(), sendToHost.getText(), 2323, loop.isSelected());
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
