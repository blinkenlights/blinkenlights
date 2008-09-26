/*
 * Created on Sep 25, 2008
 */
package de.blinkenlights.game;

import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.util.concurrent.Semaphore;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JTextField;

public class TelephoneSimulator implements UserInputSource {

    private JFrame frame;
    private volatile Character lastKey = null;
    private Semaphore gameStartSemaphore = new Semaphore(0);
    private volatile boolean offhook;
    
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
        
        JTextField instructions = new JTextField("Click here then type keys to send input to the game!");
        instructions.setEditable(false);
        instructions.addKeyListener(new KeyListener() {

            public void keyPressed(KeyEvent e) {
                // don't care
            }

            public void keyReleased(KeyEvent e) {
                System.out.println("Got key");
                lastKey = e.getKeyChar();
                e.consume();
            }

            public void keyTyped(KeyEvent e) {
                // don't care
            }
            
        });
        
        frame.add(startCallButton);
        frame.add(endCallButton);
        frame.add(instructions);
        frame.pack();
        frame.setVisible(true);
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
