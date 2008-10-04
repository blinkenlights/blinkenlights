package jmo.breakout;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Point;
import java.util.ArrayList;
import java.util.List;

public class Block implements BreakoutObject {

	public static final int BLOCK_HEIGHT = 0;
	public static final int BLOCK_WIDTH = 3;
	
	private int x;
	private int y;
	
	private Color color;
	
	List<Point> points = new ArrayList<Point>();
	
	public Block(int x, int y, Color c) {
		this.x = x;
		this.y = y;
		color = c;
		for (int i = 0; i < BLOCK_WIDTH; i++) {
			points.add(new Point(x + i, y));
		}
	}
	
	public int getHeight() {
		return BLOCK_HEIGHT;
	}
	public int getWidth() {
		return BLOCK_WIDTH;
	}
	public void move(int x, int y) {
		// Blocks can't move! Sorry!
	}
	public void paint(Graphics g) {
		g.setColor(color);
		for (Point p: points) {
			g.drawLine(p.x, p.y, p.x, p.y);
		}
		g.setColor(Breakout.DEFAULT_OBJECT_COLOR);
	}
	
	@Override
	public boolean equals(Object obj) {
		if (obj instanceof Block) {
			Block other = (Block) obj;
			
			if (other == this) {
				return true;
			}
			if (other.x == this.x && other.y == this.y) {
				return true;
			}
		}
		return false;
	}

	@Override
	public int hashCode() {
		int result = 17;
		result = 37 * result + x;
		result = 37 * result + y;
		return result;
	}
	
	public List<Point> getPoints() {
		return points;
	}
}
