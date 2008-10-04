package spaceInvaders;

import java.awt.Graphics;
import java.awt.Point;
import java.util.ArrayList;
import java.util.List;

/**
 * Players ship.
 */
public class Ship {
	
	/**
	 * The location of the player's ship in pixels starting from the far left.
	 */
	private int location;
	private SpaceInvaders game;
	
	public Ship(SpaceInvaders game) {
		this.game = game;
		location = game.getMaxHeight() / 2;
	}
	
	public void moveLeft() {
		if (location > 3) {
			location--;
		}
	}
	
	public void moveRight() {
		if (location < game.getMaxHeight() - 2) {
			location++;
		}
	}
	
	public int getLocation() {
		return location;
	}
	
	public void paint(Graphics g) {
		g.drawLine(game.getMaxWidth()-1, location - 1, game.getMaxWidth() - 2,  location);
		g.drawLine(game.getMaxWidth() - 2, location, game.getMaxWidth() - 1, location + 1);
	}
}
