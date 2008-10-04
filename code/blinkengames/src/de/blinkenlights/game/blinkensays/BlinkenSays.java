/* 
 * This file is part of BMix.
 *
 *    BMix is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 * 
 *    BMix is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with BMix.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */
package de.blinkenlights.game.blinkensays;

import java.awt.Color;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;

import de.blinkenlights.game.BlinkenGame;
import de.blinkenlights.game.FrameInfo;
import de.blinkenlights.game.GameConfigurationException;
import de.blinkenlights.game.GameContext;

/**
 * Simple example "game" that demonstrates how to use the Blinkengame API.
 * <p>
 * To launch this game, compile it:
 * 
 * <pre>
 *  % javac -cp blinkengame.jar ExampleGame.java
 * </pre>
 * 
 * then run it like this:
 * 
 * <pre>
 *  % java -cp blinkengame.jar:. ExampleGame
 * </pre>
 * 
 * (on windows, substitute a semilcolon for the colon).
 */
public class BlinkenSays implements BlinkenGame
{
	GameContext context;
	PatternManager patternManager;
	InputManager inputManager;

	boolean acceptingInput = false;
	long lastWhen;
	long failWhen;
	
	long frameDuration = 1000;

	public void gameStarting(GameContext gameContext)
	{
		System.out.println("Game starting");
		context = gameContext;

		SimonRenderer renderer = new SimonRenderer(context);
		patternManager = new PatternManager(this, renderer);
		inputManager = new InputManager(this, patternManager, renderer);

		lastWhen = 0;

		patternManager.prepareNextPattern(lastWhen);

		acceptingInput = false;
	}

	public boolean nextFrame(Graphics2D g, FrameInfo frameInfo)
	{
		if (!inputManager.gameOver())
		{
			if (acceptingInput)
			{
				inputManager.nextFrame(g, frameInfo);
			} else
			{
				patternManager.nextFrame(g, frameInfo);
			}
			lastWhen = frameInfo.getWhen();
			failWhen = lastWhen;
		}
		else
		{
			if (frameInfo.getWhen()-failWhen < frameDuration)
			{
				g.drawLine(0, 0, context.getPlayfieldWidth(), context.getPlayfieldHeight());
				g.drawLine(0, context.getPlayfieldHeight(), context.getPlayfieldWidth(), 0);
			}
			else
			{
				return false;
			}
		}
		// quit after 60 seconds so people can't hog it
		return frameInfo.getWhen() < 60000;
	}

	public void setAcceptingInput(boolean accepting)
	{
		acceptingInput = accepting;

		if (accepting)
			inputManager.prepareForInput(lastWhen);
		else
			patternManager.prepareNextPattern(lastWhen);
	}

	public void gameEnding(Graphics2D g)
	{
		System.out.println("Game ending");
		
		g.setColor(Color.white);
		g.setFont(Font.decode("System-10"));
		
		g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_OFF);
		FontMetrics fm = g.getFontMetrics();
		String s = Integer.toString(inputManager.getScore());
		
		int h = (int) fm.getAscent();
		int w = (int) fm.getStringBounds(s, g).getWidth();
		
		int centerX = context.getPlayfieldWidth() / 2;
        int centerY = context.getPlayfieldHeight() / 2;
		
		g.drawString(s, centerX - w/2, centerY+h/2);
		
		context = null;
	}

	public static void main(String[] args) throws GameConfigurationException
	{
		BlinkenGame game = new BlinkenSays();
		GameContext context = new GameContext(game);
		context.start();
	}

}