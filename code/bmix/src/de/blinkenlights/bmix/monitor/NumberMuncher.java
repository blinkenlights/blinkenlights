/*
 * Created on Aug 27, 2008
 *
 * This code belongs to SQL Power Group Inc.
 */
package de.blinkenlights.bmix.monitor;

public interface NumberMuncher {

    /**
     * Asks this muncher to incorporate the given value.
     */
    public abstract float putValue(float v);

    /**
     * Returns this muncher's current value.
     */
    public abstract float getCurrentValue();

    /**
     * Resets this muncher to the state it was in before any values were added.
     */
    public abstract void flush();

}