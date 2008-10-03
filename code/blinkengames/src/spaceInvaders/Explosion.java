package spaceInvaders;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Point;

public class Explosion {
	
	private Point loc;
	
	private enum Stages {START, STAGE1, STAGE2, DONE}
	
	private Stages stage;  
	
	public Explosion(Point p) {
		loc = p;
		stage = Stages.START;
	}
	
	public boolean isDone() {
		return stage == Stages.DONE;
	}
	
	public void paint(Graphics g) {
		if (stage == Stages.START) {
			g.drawLine(loc.x, loc.y, loc.x, loc.y);
			stage = Stages.STAGE1;
		} else if (stage == Stages.STAGE1) {
			Color c = g.getColor();
			g.setColor(new Color(0xaaaaaa));
			g.drawLine(loc.x - 1, loc.y - 1, loc.x - 1, loc.y - 1);
			g.drawLine(loc.x + 1, loc.y + 1, loc.x + 1, loc.y + 1);
			g.drawLine(loc.x + 1, loc.y - 1, loc.x + 1, loc.y - 1);
			g.drawLine(loc.x - 1, loc.y + 1, loc.x - 1, loc.y + 1);
			g.setColor(c);
			stage = Stages.STAGE2;
		} else if (stage == Stages.STAGE2) {
			Color c = g.getColor();
			g.setColor(new Color(0x777777));
			g.drawLine(loc.x - 2, loc.y - 2, loc.x - 2, loc.y - 2);
			g.drawLine(loc.x + 2, loc.y + 2, loc.x + 2, loc.y + 2);
			g.drawLine(loc.x + 2, loc.y - 2, loc.x + 2, loc.y - 2);
			g.drawLine(loc.x - 2, loc.y + 2, loc.x - 2, loc.y + 2);
			g.setColor(c);
			stage = Stages.DONE;
		}
	}
}
