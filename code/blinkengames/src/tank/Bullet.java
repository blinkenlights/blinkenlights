package tank;

import java.awt.Graphics;
import java.awt.Point;

public class Bullet {

	public static final int BULLET_SPEED = 1;
	private final String cannonDirection;
	private theTank owner;
	
	private Point location;
	
	public Bullet(Point start, String direction, theTank owner) {
		location = start;
		cannonDirection = direction;
		this.owner = owner;
	}
	
	public void moveBullet() {
		
		if(cannonDirection.equals("left")){
			location.x = location.x - BULLET_SPEED;
		}
		else if(cannonDirection.equals("up")){
			location.y = location.y - BULLET_SPEED;
		}
		
		else if(cannonDirection.equals("down")){
			location.y = location.y + BULLET_SPEED;
		}
		
		else if(cannonDirection.equals("right")){
			location.x = location.x + BULLET_SPEED;
		}
		else {
			System.out.println("impossible move");
		}
		
	}
	
	public Point getLocation() {
		return location;
	}
	
	public theTank getOwner() {
		return owner;
	}

	public void paint(Graphics g) {
		g.drawLine(location.x, location.y, location.x, location.y);		
	}

}
