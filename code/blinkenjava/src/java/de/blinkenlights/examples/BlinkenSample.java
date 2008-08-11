/**
 * BlinkenSample - A simple example of a non-interactive Blinkenlet.
 * @author Enno Brehm, Miriam Busch
 */
package de.blinkenlights.examples;

import de.blinkenlights.blinkenlet.Blinkenlet;
import de.blinkenlights.blinkenlet.BlinkenBuffer;
import de.blinkenlights.blinkenlet.BlinkenEvent;

public class BlinkenSample extends Blinkenlet {
    int w;
    int h;
    int max;

    int x = 0;
    int y = 0;

    public BlinkenSample() {
        super("BlinkenSample",
        "Enno Brehm, Miriam Busch",
        "A simple example of a non-interactive Blinkenlet.");
    }

    public boolean prepare(int width, int height, int channels, int maxVal) {
        w = width;
        h = height;
        this.max = maxVal;
	System.out.println("max = " + max);
        return true;
    }

    public void start(BlinkenBuffer buffer) {
        startTicker(0);
    }

    public int tick(BlinkenBuffer buffer) {
        buffer.setPixel(x, y, (byte)max);
        if( ++x == w ) {
            x = 0;
            if( ++y == h ) {
                /* we want to stop, but we can not ... */
		requestStop();
                y = 0;
            }
        }
        return 100;
    }

    public void handleEvent(BlinkenEvent event, BlinkenBuffer buffer) {
    }
}
