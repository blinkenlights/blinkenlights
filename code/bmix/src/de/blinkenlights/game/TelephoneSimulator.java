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
import javax.swing.JLabel;

public class TelephoneSimulator implements UserInputSource {

    private JFrame frame;
    private Character lastKey = null;
    private Semaphore gameStartSemaphore = new Semaphore(0);
    
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
                gameStartSemaphore.release();
            }
        });
        
        JButton endCallButton = new JButton("End Call");
        // TODO end call
        
        JLabel instructions = new JLabel("Click here then type keys to send input to the game!");
        
        frame.addKeyListener(new KeyListener() {

            public void keyPressed(KeyEvent e) {
                // don't care
            }

            public void keyReleased(KeyEvent e) {
                // don't care
            }

            public void keyTyped(KeyEvent e) {
                System.out.println("Got key");
                lastKey = e.getKeyChar();
                e.consume();
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

    
}
