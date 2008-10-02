/* 
 * This file is part of BMix.
 *
 *    BMix is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 * 
 *    BMix is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with BMix.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */
package de.blinkenlights.sender;

import java.awt.FileDialog;
import java.awt.Frame;
import java.awt.event.ActionEvent;
import java.io.File;

import javax.swing.AbstractAction;

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
