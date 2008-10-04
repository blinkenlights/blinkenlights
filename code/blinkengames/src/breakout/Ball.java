package breakout;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Point;

public class Ball {

	private Point loc;
	private Point lastLoc;
	private Point lastLastLoc;
	private Breakout game;
	
	private boolean moveLeft;
	private boolean moveUp;
	
	/**
	 * Flag for cutting ball motion rate in half.
	 */
    private boolean moveNextTime;
	
	public Ball(Breakout parent, Point start) {
		loc = start;
		lastLoc = start;
		lastLastLoc = start;
		game = parent;
		moveLeft = true;
		moveUp = true;
	}
	
	public void move() {
	    moveNextTime = !moveNextTime;
	    if (moveNextTime) {
	        return;
	    }
		lastLastLoc = new Point(lastLoc);
		lastLoc = new Point(loc);
		if (moveUp) {
			loc.y--;
		} else {
			loc.y++;
		}
		if (moveLeft) {
			loc.x--;
		} else {
			loc.x++;
		}
		
		if (loc.y == 0) {
			moveUp = false;
		} else if (loc.y == game.getMaxHeight() - 1) {
			moveUp = true;
		}
		if (loc.x == 0) {
			moveLeft = false;
		} else if (loc.x == game.getMaxWidth()) {
			moveLeft = true;
		}
	}
	
	public void paddleHit() {
		moveLeft = true;
	}
	
	public Point getLocation() {
		return loc;
	}
	
	public void paint(Graphics2D g) {
		g.setColor(new Color(0x444444));
		g.drawLine(lastLastLoc.x, lastLastLoc.y, lastLastLoc.x, lastLastLoc.y);
		g.setColor(new Color(0x999999));
		g.drawLine(lastLoc.x, lastLoc.y, lastLoc.x, lastLoc.y);
		g.setColor(new Color(0xffffff));
		g.drawLine(loc.x, loc.y, loc.x, loc.y);
	}

	public void brickHit() {
		moveLeft = false;
	}

	public void brickHitFromBelow() {
		moveUp = false;
	}
	
	public void brickHitFromAbove() {
		moveUp = true;
	}
}
