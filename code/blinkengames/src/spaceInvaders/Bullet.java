package spaceInvaders;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Point;

public class Bullet {

	public static final int BULLET_SPEED = 2;
	
	private Point location;
	
	public Bullet(Point start) {
		location = start;
	}
	
	public void moveBullet() {
		location.x = location.x - BULLET_SPEED;
	}
	
	public Point getLocation() {
		return location;
	}

	public void paint(Graphics g) {
		g.drawLine(location.x, location.y, location.x, location.y);
		g.setColor(new Color(0x999999));
		g.drawLine(location.x+1, location.y, location.x+1, location.y);
		g.setColor(new Color(0x444444));
		g.drawLine(location.x+2, location.y, location.x+2, location.y);
		g.setColor(new Color(0xffffff));
	}

}
