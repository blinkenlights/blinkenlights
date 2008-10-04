package de.blinkenlights.game.blinkensays;

import java.awt.Color;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;

import de.blinkenlights.game.GameContext;

public class SimonRenderer
{
	int centerX, centerY;
	GameContext context;
	Font font;
	
	public SimonRenderer(GameContext gameContext)
	{
		context = gameContext;
        centerX = gameContext.getPlayfieldWidth() / 2;
        centerY = gameContext.getPlayfieldHeight() / 2;
        
        font = Font.decode("System-8");
	}

	void render(Graphics2D g, int currNumber)
	{
		g.setColor(Color.white);
		g.setFont(font);
		
		g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_OFF);
		FontMetrics fm = g.getFontMetrics();
		int h = (int) fm.getAscent();
		int w;
		String s;
		
		switch(currNumber)
		{
		case 1:
			g.fillRect(0, 0, centerX, centerY);
			g.setColor(Color.black);
			
	        s = "1";
	        w = (int) fm.getStringBounds(s, g).getWidth();
	        g.drawString(s, centerX/2 - w/2, centerY);
	        
			break;
		case 2:
			g.fillRect(centerX, 0, centerX, centerY);
			g.setColor(Color.black);
			
			s = "2";
	        w = (int) fm.getStringBounds(s, g).getWidth();
	        g.drawString(s, (centerX*3/2) - w/2, centerY);
			
			break;
		case 3:
			g.fillRect(0, centerY, centerX, centerY);
			g.setColor(Color.black);

			s = "4";
	        w = (int) fm.getStringBounds(s, g).getWidth();
	        g.drawString(s, centerX/2 - w/2, centerY + h-2);			
			
			break;
		case 4:
			g.fillRect(centerX, centerY, centerX, centerY*2);
			g.setColor(Color.black);
			
			s = "5";
	        w = (int) fm.getStringBounds(s, g).getWidth();
	        g.drawString(s, (centerX*3/2) - w/2, centerY*2);
			
			break;
		}
	}
	
}
