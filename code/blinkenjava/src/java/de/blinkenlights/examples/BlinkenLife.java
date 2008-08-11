/*
 * Created by IntelliJ IDEA.
 * User: enno
 * Date: Sep 13, 2002
 * Time: 1:28:22 PM
 * To change template for new class use
 * Code Style | Class Templates options (Tools | IDE Options).
 */
package de.blinkenlights.examples;

import de.blinkenlights.blinkenlet.Blinkenlet;
import de.blinkenlights.blinkenlet.BlinkenBuffer;
import de.blinkenlights.blinkenlet.BlinkenEvent;
import de.blinkenlights.blinkenlet.BlinkenKeyEvent;

public class BlinkenLife extends Blinkenlet {
    Life life;
    int origMaxVal;

    int maxVal;
    int r;

    public BlinkenLife() {
        super("BlinkenLife",
        "Enno Brehm",
        "A Game of Life."
        );
    }

    public boolean prepare(int width, int height, int channels, int maxVal) {
        this.origMaxVal = maxVal;
        r = maxVal / 8;

        /* to stop "aging" use
              new Life(width, height, maxVal);
           instead
         */
        life = new Life(width, height, maxVal, r);
        reset();
        return true;
    }

    void reset() {
        maxVal = origMaxVal;
        life.randomize(0.4);
    }

    public void start(BlinkenBuffer buffer) {
        startTicker(0);
    }

    public void stop(BlinkenBuffer buffer) {
    }

    public int tick(BlinkenBuffer buffer) {
        if (maxVal == 0) {
            reset();
        }

        final int[] cells = life.getCells();
        final int w = life.getWidth();
        final int h = life.getHeight();

        int index = 0;
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                int v = (cells[index++]);
                buffer.setPixel(x, y, v);
            }
        }
        if (!life.nextGeneration()) {
            maxVal = Math.max(maxVal - r, 0);
            System.out.println("no changes. reducing maxval to: " + maxVal);
        }
        buffer.paint();
        return 500;
    }

}
