package breakout;

import java.awt.Graphics2D;
import java.awt.Point;
import java.util.ArrayList;
import java.util.List;

public class Paddle {

	private Point leftMostPoint;
	
	private int length;
	
	private Breakout game;
	
	public Paddle(Breakout parent, Point start, int startLength) {
		leftMostPoint = start;
		length = startLength;
		game = parent;
	}
	
	public void move(boolean up) {
		if (up && (leftMostPoint.y - length) >= 0) {
			leftMostPoint.y--;
		} else if (!up && (leftMostPoint.y) < game.getMaxHeight() - 1) {
			leftMostPoint.y++;
		}
	}
	
	public List<Point> getPaddlePoints() {
		List<Point> newList = new ArrayList<Point>();
		for (int i = 0; i < length; i++) {
			newList.add(new Point(leftMostPoint.x, leftMostPoint.y-i));
		}
		return newList;
	}
	
	public void paint(Graphics2D g) {
		g.drawLine(leftMostPoint.x, leftMostPoint.y, leftMostPoint.x, leftMostPoint.y-length+1);
	}
	                           
}
