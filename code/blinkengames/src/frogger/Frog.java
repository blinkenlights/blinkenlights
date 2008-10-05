/*
 * Created on Sep 29, 2008
 *
 * This code belongs to SQL Power Group Inc.
 */
package frogger;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Point;

public class Frog {
    
    public enum Direction {LEFT, RIGHT, UP, DOWN};
    
    private Point frogHead;
    
    private Frogger game;
    
    public Frog(Frogger parent, Point start) {
        game = parent;
        frogHead = start;
    }
    
    public void move(Direction d) {
        if (d.equals(Direction.LEFT)) {
            frogHead.x -= game.getStreamWidth();
        } else if (d.equals(Direction.RIGHT)) {
            frogHead.x += game.getStreamWidth();
        } else if (d.equals(Direction.UP)) {
            frogHead.y -= 1;
        } else if (d.equals(Direction.DOWN)) {
            frogHead.y += 1;
        }
    }
    
    public int getFrogLeftEdge() {
        return frogHead.x;
    }
    
    public void paint(Graphics2D g) {
        int x = frogHead.x;
        int y = frogHead.y;
        g.setColor(new Color(0x999999));
        g.drawLine(x, y, x, y);
        g.drawLine(x+1, y-1, x+2, y-1);
        g.drawLine(x+1, y+1, x+2, y+1);
        g.drawLine(x, y-2, x, y-2);
        g.drawLine(x, y+2, x, y+2);
        g.drawLine(x+2, y-2, x+2, y-2);
        g.drawLine(x+2, y+2, x+2, y+2);
    }

    public int getTopEdge() {
        return frogHead.y-2;
    }

    public int getBottomEdge() {
        return frogHead.y+2;
    }
    
    public void setFrogHead(Point frogHead) {
        this.frogHead = frogHead;
    }

}
