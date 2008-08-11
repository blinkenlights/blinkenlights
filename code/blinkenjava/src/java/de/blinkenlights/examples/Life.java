/*
 * Created by IntelliJ IDEA.
 * User: enno
 * Date: Aug 1, 2002
 * Time: 8:29:52 PM
 * To change template for new class use
 * Code Style | Class Templates options (Tools | IDE Options).
 */
package de.blinkenlights.examples;

import java.util.*;

/**
 * This is a utility class used by BlinkenLife
 */

class Life {
    int width;
    int height;
    int[] cells;
    /** generation counter */
    int generations = 0;

    int maxVal = 1;
    int aging = 0;

    /** for initialization purposes */
    static private Random rnd = new Random();

    /* temporary storage for calculation of next generation */
    private TmpCells tmp;


    public Life(int w, int h) {
        this(w, h, new int[w * h], 1, 0);
    }

    public Life(int w, int h, int maxVal) {
        this(w, h, new int[w * h], maxVal, 0);
    }

    public Life(int w, int h, int maxVal, int aging) {
        this(w, h, new int[w * h], maxVal, aging);
    }


    Life(int w, int h, int[] field, int maxVal, int aging) {
        this.width = w;
        this.height = h;
        this.cells = field;
        this.aging = aging;
        this.maxVal = maxVal;
        tmp = new TmpCells();
    }

    public int getWidth() {
        return width;
    }

    public int getHeight() {
        return height;
    }

    public int[] getCells() {
        return cells;
    }


    /** initialize a playfield with random values
     *
     * @param fraction fraction of pixels that should become set (on average).
     * Lower values result in playfield with few pixels set, higher values will
     * cause many pixels to be set.
     */
    public void randomize(double fraction) {
        int idx = 0;
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                cells[idx++] = (rnd.nextFloat() < fraction) ? maxVal : 0;
                //System.out.println("cells[idx] = " + cells[idx - 1]);
            }
        }
    }

    /** let the game proceed one generation.
     * @return true if any change happened at all
     */
    public boolean nextGeneration() {
        boolean changed = false;
        tmp.init();

        int index = 0;
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                int oldValue = tmp.get(j, i);
                int newValue;

                /* calculate number of neighbours */
                int nb = tmp.getNeighbourCount(j, i);

                switch (nb) {
                    case 3:
                        if (oldValue == 0) {
                            newValue = maxVal;
                            break;
                        }
                        /* fallthrough */
                    case 2:
                        newValue = oldValue; // survives
                        if (oldValue > 0) {
                            newValue = Math.max(maxVal/2, newValue - aging);
                        }
                        break;

                    default:
                        newValue = 0;
                }
                //System.out.println("x=" + j + " y=" + i + " nb = " + nb + " newval=" + newValue);
                cells[index++] = newValue;
                if (newValue != oldValue) {
                    changed = true;
                }
            }
        }
        generations++;
        return changed;
    }


    /**
     * retrieve the number of generations this game has calculated
     *
     * @return the age of this game
     */
    public int getGenerations() {
        return generations;
    }

    class TmpCells {
        int tmpcells[][];

        TmpCells() {
            tmpcells = new int[width][height];
            init();
        }

        public void init() {
            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                    tmpcells[j][i] = cells[i * width + j];
                }
            }
        }

        int get(int x, int y) {
            if (x < 0 || x >= width) return 0;
            if (y < 0 || y >= height) return 0;
            return tmpcells[x][y];
        }

        private int getNeighbourCount(int x, int y) {
            int nb = 0;
            for (int dx = -1; dx < 2; dx++) {
                for (int dy = -1; dy < 2; dy++) {
                    if (dx == 0 && dy == 0) continue;
                    if (get(x + dx, y + dy) != 0)
                        nb++;
                }
            }
            return nb;
        }

    }


}

