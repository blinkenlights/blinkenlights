/*
 * Created on Aug 27, 2008
 *
 * This code belongs to SQL Power Group Inc.
 */
package de.blinkenlights.bmix.monitor;

public class PeriodicAverage implements NumberMuncher {

    private final NumberMuncher muncher;
    private float currentValue;
    private long nextFlushTime;
    private final long interval;
    
    public PeriodicAverage(NumberMuncher muncher, long interval) {
        this.muncher = muncher;
        this.interval = interval;
        nextFlushTime = System.currentTimeMillis();
    }
    
    public float getCurrentValue() {
        long now = System.currentTimeMillis();
        if (now >= nextFlushTime) {
            currentValue = muncher.getCurrentValue();
            muncher.flush();
            nextFlushTime += interval;
        }
        return currentValue;
    }

    public float putValue(float v) {
        muncher.putValue(v);
        return getCurrentValue();
    }

    public void flush() {
        muncher.flush();
        nextFlushTime = System.currentTimeMillis();
    }

}
