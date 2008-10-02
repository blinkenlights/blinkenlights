/*
 * Created on Sep 25, 2008
 */
package de.blinkenlights.game;

import java.awt.FlowLayout;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.concurrent.Semaphore;

import javax.swing.AbstractAction;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;

public class TelephoneSimulator implements UserInputSource {

    private JFrame frame;
    private volatile Character lastKey = null;
    private Semaphore gameStartSemaphore = new Semaphore(0);
    private volatile boolean offhook;
    
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
        frame.setLocationByPlatform(true);
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setLayout(new FlowLayout(FlowLayout.CENTER));
        
        JButton startCallButton = new JButton("Start Call");
        startCallButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                lastKey = null;
                offhook = true;
                gameStartSemaphore.release();
            }
        });
        
        JButton endCallButton = new JButton("End Call");
        endCallButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                offhook = false;
            }
        });
        
        frame.add(startCallButton);
        frame.add(endCallButton);
        frame.add(makeKeyPad());
        frame.pack();
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
    }

    public boolean isUserPresent() {
        return offhook;
    }

    
}
