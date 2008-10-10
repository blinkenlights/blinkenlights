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
package de.blinkenlights.game;

import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.concurrent.Semaphore;

import javax.swing.AbstractAction;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;

public class TelephoneSimulator implements UserInputSource {

	private static final String MUSIC_LABEL_PREFIX = "Current background music: ";
    private JFrame frame;
    private volatile Character lastKey = null;
    private Semaphore gameStartSemaphore = new Semaphore(0);
    private volatile boolean offhook;
    private JLabel statusLabel = new JLabel();
    private String musicName;
    
    private class NumberAction extends AbstractAction {
        
        private final char digit;
        
        public NumberAction(char digit) {
            super(String.valueOf(digit));
            this.digit = digit;
        }
        
        public void actionPerformed(ActionEvent e) {
            lastKey = digit;
        }
    }
    
    public Character getKeystroke() {
        Character retval = lastKey;
        lastKey = null;
        return retval;
    }

    public void start() {
        frame = new JFrame("Telephone!");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setLayout(new BorderLayout());
        
        JButton startCallButton = new JButton("Start Call");
        startCallButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                lastKey = null;
                offhook = true;
                gameStartSemaphore.release();
                updateStatus();
            }
        });
        
        JButton endCallButton = new JButton("End Call");
        endCallButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                offhook = false;
                updateStatus();
            }
        });
        JPanel buttonPanel = new JPanel();
        buttonPanel.add(startCallButton);
        buttonPanel.add(endCallButton);
        
        frame.add(buttonPanel,BorderLayout.WEST);
        frame.add(makeKeyPad(),BorderLayout.CENTER);
        frame.add(statusLabel,BorderLayout.SOUTH);
        frame.pack();
        frame.setLocation(400, 100);
        frame.setVisible(true);
    }

    private JPanel makeKeyPad() {
        JPanel p = new JPanel(new GridLayout(4, 3));
        p.add(new JButton(new NumberAction('1')));
        p.add(new JButton(new NumberAction('2')));
        p.add(new JButton(new NumberAction('3')));
        p.add(new JButton(new NumberAction('4')));
        p.add(new JButton(new NumberAction('5')));
        p.add(new JButton(new NumberAction('6')));
        p.add(new JButton(new NumberAction('7')));
        p.add(new JButton(new NumberAction('8')));
        p.add(new JButton(new NumberAction('9')));
        p.add(new JButton(new NumberAction('*')));
        p.add(new JButton(new NumberAction('0')));
        p.add(new JButton(new NumberAction('#')));
        return p;
    }
    
    public void stop() {
        frame.dispose();
    }

    public void waitForGameStart() throws InterruptedException {
        gameStartSemaphore.acquire();
    }

    public void gameEnding() {
        offhook = false;
        musicName = null;
        lastKey = null;
        gameStartSemaphore.drainPermits();
        updateStatus();
    }

    public boolean isUserPresent() {
        return offhook;
    }

	public void playBackgroundMusic(String musicName) {
        this.musicName = musicName;
        updateStatus();
	}

	private void updateStatus() {
	    statusLabel.setText("Offhook: " + offhook + " " + MUSIC_LABEL_PREFIX + musicName);
	}
    
}
