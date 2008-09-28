/*
 * Created on Sep 25, 2008
 */
package de.blinkenlights.sender;

import java.awt.FileDialog;
import java.awt.Frame;
import java.awt.event.ActionEvent;
import java.io.File;

import javax.swing.AbstractAction;
import javax.swing.JOptionPane;

public class OpenFileAction extends AbstractAction {

    private final Frame parentFrame;
    private final Sender sender;

    public OpenFileAction(Frame parentFrame, Sender sender) {
        super("Open...");
        this.parentFrame = parentFrame;
        this.sender = sender;
    }
    
    public void actionPerformed(ActionEvent e) {
        FileDialog fd = new FileDialog(parentFrame, "Choose a BML File");
        fd.setVisible(true);
        String dir = fd.getDirectory();
        String file = fd.getFile();
        if (dir != null && file != null) {
					File chosenFile = new File(dir, file);
					sender.sendMovie(chosenFile);
				}
    }
}
