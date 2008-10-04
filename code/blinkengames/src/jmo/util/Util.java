package jmo.util;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Point;

public class Util {

	public static void paintGameOver(Graphics g, int score) {
		g.setColor(new Color(0xffffff));
		paintG(g, new Point(0, 0));
		paintA(g, new Point(4, 0));
		paintM(g, new Point(8, 0));
		paintE(g, new Point(14, 0));
		
		g.setColor(new Color(0xaaaaaa));
		paintO(g, new Point(5, 4));		
		paintV(g, new Point(9, 4));
		paintE(g, new Point(13, 4));
		paintR(g, new Point(17, 4));
		
		g.setColor(new Color(0xffffff));
		paintS(g, new Point(0, 8));
		paintC(g, new Point(4, 8));
		paintO(g, new Point(8, 8));
		paintR(g, new Point(12, 8));
		paintE(g, new Point(16, 8));
		
		paintNumber(g, new Point(21, 8), (score/10)%10);
		paintNumber(g, new Point(25, 8), score%6);
	}
	
	private static void paintC(Graphics g, Point point) {
		int x = point.x;
		int y = point.y;
		g.drawLine(x, y, x, y+3);
		g.drawLine(x+1, y, x+2, y);
		g.drawLine(x+1, y+3, x+2, y+3);
	}

	private static void paintS(Graphics g, Point point) {
		int x = point.x;
		int y = point.y;
		g.drawLine(x, y-1, x+2, y-1);
		g.drawLine(x, y, x, y);
		g.drawLine(x, y+1, x+2, y+1);
		g.drawLine(x+2, y+2, x+2, y+2);
		g.drawLine(x, y+3, x+2, y+3);
	}

	public static void paintG(Graphics g, Point p) {
		int x = p.x;
		int y = p.y;
		g.drawLine(x, y, x, y+3);
		g.drawLine(x+1, y+3, x+1, y+3);
		g.drawLine(x+2, y+3, x+2, y+2);
		g.drawLine(x+1, y, x+2, y);
	}
	
	public static void paintA(Graphics g, Point p) {
		int x = p.x;
		int y = p.y;
		g.drawLine(x, y+3, x, y+1);
		g.drawLine(x+1, y, x+1, y);
		g.drawLine(x+2, y+3, x+2, y+1);
		g.drawLine(x+1, y+2, x+1, y+2);
	}
	
	public static void paintM(Graphics g, Point p) {
		int x = p.x;
		int y = p.y;
		g.drawLine(x, y+3, x, y+1);
		g.drawLine(x+1, y, x+1, y);
		g.drawLine(x+2, y+3, x+2, y+1);
		g.drawLine(x+3, y, x+3, y);
		g.drawLine(x+4, y+3, x+4, y+1);
	}
	
	public static void paintE(Graphics g, Point p) {
		int x = p.x;
		int y = p.y;
		g.drawLine(x, y, x, y+3);
		g.drawLine(x+1, y, x+2, y);
		g.drawLine(x+1, y+2, x+1, y+2);
		g.drawLine(x+1, y+3, x+2, y+3);
	}
	
	public static void paintO(Graphics g, Point p) {
		int x = p.x;
		int y = p.y;
		g.drawLine(x, y+1, x, y+2);
		g.drawLine(x+1, y, x+1, y);
		g.drawLine(x+2, y+1, x+2, y+2);
		g.drawLine(x+1, y+3, x+1, y+3);
	}
	
	public static void paintV(Graphics g, Point p) {
		int x = p.x;
		int y = p.y;
		g.drawLine(x, y, x, y+2);
		g.drawLine(x+1, y+3, x+1, y+3);
		g.drawLine(x+2, y, x+2, y+2);
	}
	
	public static void paintR(Graphics g, Point p) {
		int x = p.x;
		int y = p.y;
		g.drawLine(x, y, x, y+3);
		g.drawLine(x+1, y, x+1, y);
		g.drawLine(x+2, y+1, x+2, y+1);
		g.drawLine(x+1, y+2, x+1, y+2);
		g.drawLine(x+2, y+3, x+2, y+3);
	}
	
	public static void paint1(Graphics g, Point p) {
		int x = p.x;
		int y = p.y;
		g.drawLine(x+2, y, x+2, y+3);
	}
	
	public static void paint2(Graphics g, Point p) {
		int x = p.x;
		int y = p.y;
		g.drawLine(x, y-1, x+2, y-1);
		g.drawLine(x+2, y, x+2, y);
		g.drawLine(x, y+1, x+2, y+1);
		g.drawLine(x, y+2, x, y+2);
		g.drawLine(x, y+3, x+2, y+3);
	}
	public static void paint3(Graphics g, Point p) {
		int x = p.x;
		int y = p.y;
		g.drawLine(x+2, y-1, x+2, y+3);
		g.drawLine(x, y-1, x+1, y-1);
		g.drawLine(x+1, y+1, x+1, y+1);
		g.drawLine(x, y+3, x+1, y+3);
	}
	public static void paint4(Graphics g, Point p) {
		int x = p.x;
		int y = p.y;
		g.drawLine(x, y, x, y+2);
		g.drawLine(x+1, y+2, x+1, y+2);
		g.drawLine(x+2, y, x+2, y+3);
	}

	public static void paint6(Graphics g, Point p) {
		int x = p.x;
		int y = p.y;
		g.drawLine(x, y-1, x, y+3);
		g.drawLine(x+1, y+1, x+1, y+1);
		g.drawLine(x+1, y+3, x+1, y+3);
		g.drawLine(x+2, y+1, x+2, y+3);
	}
	public static void paint7(Graphics g, Point p) {
		int x = p.x;
		int y = p.y;
		g.drawLine(x+2, y, x+2, y+3);
		g.drawLine(x, y, x+1, y);
	}
	public static void paint8(Graphics g, Point p) {
		int x = p.x;
		int y = p.y;
		g.drawLine(x, y-1, x, y+3);
		g.drawLine(x+2, y-1, x+2, y+3);
		g.drawLine(x+1, y-1, x+1, y-1);
		g.drawLine(x+1, y+1, x+1, y+1);
		g.drawLine(x+1, y+3, x+1, y+3);
	}
	public static void paint9(Graphics g, Point p) {
		int x = p.x;
		int y = p.y;
		g.drawLine(x, y, x, y+2);
		g.drawLine(x+1, y, x+1, y);
		g.drawLine(x+1, y+2, x+1, y+2);
		g.drawLine(x+2, y, x+2, y+3);
	}
	
	public static void paintNumber(Graphics g, Point p, int number) {
		if (number == 0) {
			paintO(g, p);
		} else if (number == 1) {
			paint1(g, p);
		} else if (number == 2) {
			paint2(g, p);
		} else if (number == 3) {
			paint3(g, p);
		} else if (number == 4) {
			paint4(g, p);
		} else if (number == 5) {
			paintS(g, p);
		} else if (number == 6) {
			paint6(g, p);
		} else if (number == 7) {
			paint7(g, p);
		} else if (number == 8) {
			paint8(g, p);
		} else if (number == 9) {
			paint9(g, p);
		} 
		
	}
	
}
