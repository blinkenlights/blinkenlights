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

import java.awt.image.BufferedImage;
import java.io.IOException;

public interface FrameTarget {

    /**
     * Presents the given image to the user.
     * 
     * @throws IOException If the frame can't be sent to the target
     */
    void putFrame(BufferedImage image) throws IOException;
    
    /**
     * Performs any necessary startup routine for this frame target.
     */
    public void start();
    
    /**
     * Performs the shutdown routine which releases any resources held
     * by this frame target.
     */
    public void stop();

    /**
     * Notifies this frame target that the game is over. This frame target
     * will release any resources it had been using during the game.
     */
    public void gameEnding();
}
