package jmo.breakout;

import java.awt.Graphics;

public interface BreakoutObject {
	public void move(int x, int y);
	
	public int getHeight();
	
	public int getWidth();
	
	public void paint(Graphics g);
}