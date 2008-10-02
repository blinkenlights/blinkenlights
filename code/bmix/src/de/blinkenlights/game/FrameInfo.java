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
package de.blinkenlights.game;

public class FrameInfo {

    /**
     * Number of milliseconds since the beginning of the game.
     */
    private final long when;
    
    private final Character userInput;
    
    /**
     * @param userInput
     * @param when
     */
    public FrameInfo(Character userInput, long when) {
        super();
        this.userInput = userInput;
        this.when = when;
    }
    
    public long getWhen() {
        return when;
    }
    
    public Character getUserInput() {
        return userInput;
    }
    
}
