/* 
 * This file is part of BMix.
 *
 *    BMix is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 * 
 *    BMix is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with BMix.  If not, see <http://www.gnu.org/licenses/>.
 * 
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
