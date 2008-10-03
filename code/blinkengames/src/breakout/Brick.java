package breakout;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Point;
import java.util.ArrayList;
import java.util.List;

public class Brick {
	
	public static final int DARKEST_SHADE = 5;
	
	private Point leftMostPoint;
	private int length;
	private int colour;
	private boolean darkening;
	private int darkestCurrentShade;
	
	public Brick(Point leftMost, int length) {
		this.leftMostPoint = leftMost;
		this.length = length;
		darkening = true;
		darkestCurrentShade = (int) (Math.random()*5+DARKEST_SHADE);
		colour = (int) (Math.random()*(15-darkestCurrentShade)+darkestCurrentShade+1);
	}
	
	public List<Point> getPoints() {
		List<Point> newList = new ArrayList<Point>();
		for (int i = 0; i < length; i++) {
			newList.add(new Point(leftMostPoint.x, leftMostPoint.y-i));
		}
		return newList;
	}
	
	public void paint(Graphics2D g) {
		
		if (!darkening) {
			colour++;
			if (colour == 15) {
				darkening = true;
			}
		} else if (darkening) {
			colour--;
			if( colour == darkestCurrentShade) {
				darkening = false;
			}
		}
		
		g.setColor(new Color(colour * 16, colour * 16, colour * 16));
		g.drawLine(leftMostPoint.x, leftMostPoint.y, leftMostPoint.x, leftMostPoint.y-length+1);
	}

}
