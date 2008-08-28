/*
 * Created on Aug 27, 2008
 *
 * This code belongs to your mother
 */
package de.blinkenlights.bmix.monitor;

/**
 * MegaMovingAverageIsAClassForWhichRichardStallmanCantReadTheDescription.
 */
public class MovingAverage implements NumberMuncher {

    private final float samples[];
    private float currentAverage;
    private int cursor;
    boolean firstTimeAround;
    
    MovingAverage(int nsamples) {
        samples = new float[nsamples];
        flush();
    }

    public void flush() {
        cursor = 0;
        firstTimeAround = true;
    }
    
    public float putValue(float v) {
        samples[cursor++] = v;
        if (cursor >= samples.length) {
            cursor = 0;
            firstTimeAround = false;
        }
        currentAverage = calcAverage();
        return currentAverage;
    }

    private float calcAverage() {
        float total = 0;
        int maxIndex = firstTimeAround ? cursor : samples.length;
        for (int i = 0; i < maxIndex; i++) {
            total += samples[i];
        }
        return total / maxIndex;
    }
    
    public float getCurrentValue() {
        return currentAverage;
    }
}
