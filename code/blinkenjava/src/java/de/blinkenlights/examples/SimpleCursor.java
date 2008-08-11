/*
 * SimpleCursor - A simple example for an interactive Blinkenlet.
 * This blinkenlet reacts to events only, it does not want to receive tick calls.
 * The house displays a cursor that moves via user input.
 * @author Miriam Busch
 */
package de.blinkenlights.examples;

import de.blinkenlights.blinkenlet.Blinkenlet;
import de.blinkenlights.blinkenlet.BlinkenBuffer;
import de.blinkenlights.blinkenlet.BlinkenEvent;
import de.blinkenlights.blinkenlet.BlinkenKeyEvent;

public class SimpleCursor extends Blinkenlet {
    int width;
    int height;
    int max;

    int cursorX = 0;
    int cursorY = 0;

    public SimpleCursor() {
        super("SimpleCursor",
                "Miriam Busch",
                "A simple example for an interactive Blinkenlet.");
    }

    public boolean prepare(int width, int height, int channels, int maxVal) {
        this.width = width;
        this.height = height;
        this.max = maxVal;
        return (width > 1) && (height > 1);
    }

    public void start(BlinkenBuffer buffer) {
        centerCursor();
        buffer.clear();
        buffer.setPixel(cursorX, cursorY, max);
        buffer.paint();
    }

    private void centerCursor() {
        cursorX = width / 2;
        cursorY = height / 2;
    }


    public void handleEvent(BlinkenEvent event, BlinkenBuffer buffer) {
        if (event instanceof BlinkenKeyEvent) {
            BlinkenKeyEvent keyEvent = (BlinkenKeyEvent) event;
            System.out.println("Cursor will move! "+keyEvent.getKey());
            switch (keyEvent.getKey()) {
                case BlinkenKeyEvent.KEY_1:
                    break;
                case BlinkenKeyEvent.KEY_2:
                    // up
                    cursorY--;
                    break;
                case BlinkenKeyEvent.KEY_3:
                    break;
                case BlinkenKeyEvent.KEY_4:
                    //left
                    cursorX--;
                    break;
                case BlinkenKeyEvent.KEY_6:
                    //right
                    cursorX++;
                    break;
                case BlinkenKeyEvent.KEY_8:
                    //down
                    cursorY++;
                    break;
            }
            checkOutOfBounds(buffer);
            buffer.clear();
            System.out.println("Curser at "+cursorX+", "+cursorY);
            buffer.setPixel(cursorX, cursorY, max);
            buffer.paint();
        } else {
            System.err.println("SimpleCursor: Received unknown event: " + event);
        }
    }

    static int SLEEPTIME = 75;
    private void checkOutOfBounds(BlinkenBuffer buffer) {
      try{ 
        if (cursorX < 0 || cursorX >= width || cursorY < 0 || cursorY >= height) {
            setAll(buffer, max);
            buffer.paint();
            Thread.sleep(SLEEPTIME);
            buffer.clear();
            buffer.paint();
            Thread.sleep(SLEEPTIME);
            setAll(buffer, max/2);
            buffer.paint();
            Thread.sleep(SLEEPTIME);
            centerCursor();
        }
      }catch( InterruptedException ie) {
        System.err.println("" + ie);
      }
    }

    private void setAll(BlinkenBuffer buffer, int color) {
        for (int i=0; i < width; i++){
            for (int j=0; j < height; j++) {
                buffer.setPixel(i,j,color);
            }
        }
    }
}
