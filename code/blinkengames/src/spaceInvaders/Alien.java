package spaceInvaders;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Point;


/**
 * Aliens!!! Rawr
 */
public class Alien {
	
	public static final int DARKEST_SHADE = 8;
	
	private Point loc;
	
	/**
	 * Should only go from 0 to 15 although 15 probably isn't very visible.
	 */
	private int colour;
	
	private boolean darkening;
	
	private boolean moveLeft;
	
	private boolean moveDown;

	private SpaceInvaders game;
	
	private int maxDelay;
	
	private int moveCountdown;
	
	public Alien(Point p, SpaceInvaders game, int delay) {
		this.game = game;
		loc = p;
		colour = 15;
		darkening = true;
		moveDown = false;
		moveLeft = false;
		maxDelay = delay;
		moveCountdown = delay;
	}
	
	public Point getLocation() {
		return loc;
	}
	
	public void paint(Graphics g) {
		if (!darkening) {
			colour++;
			if (colour == 15) {
				darkening = true;
			}
		} else if (darkening) {
			colour--;
			if( colour == DARKEST_SHADE) {
				darkening = false;
			}
		}
		
		g.setColor(new Color(colour * 16, colour * 16, colour * 16));
		g.drawLine(loc.x, loc.y, loc.x-1, loc.y);
	}
	
	public boolean move(boolean switchDirection) {
		
		moveCountdown--;
		if (moveCountdown <= 0) {
			if (switchDirection && !moveDown) {
				loc.x++;
				moveLeft = !moveLeft;
				moveDown = true;
			} else if (moveLeft) {
				loc.y--;
				moveDown = false;
			} else {
				loc.y++;
				moveDown = false;
			}
			
			moveCountdown = maxDelay;
		}
		
		if (loc.y == 0 || loc.y == game.getMaxHeight() - 1) {
			return true;
		}
		
		return false;
	}
	
}
