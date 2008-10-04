package de.blinkenlights.game.blinkensays;

import java.awt.Graphics2D;

import de.blinkenlights.game.FrameInfo;

public class InputManager
{
	SimonRenderer renderer;
	PatternManager patternManager;
	BlinkenSays game;

	int lastEnteredVal = -1;
	int inputIndex = 0;
	long lastInputWhen = 0;
	long frameDuration = 1000;
	
	boolean gameOver;
	int score;

	public InputManager(BlinkenSays game, PatternManager patternManager,
			SimonRenderer renderer)
	{
		this.game = game;
		this.patternManager = patternManager;
		this.renderer = renderer;
		this.score = 0;
		this.gameOver = false;
	}

	public void prepareForInput(long lastWhen)
	{
		lastEnteredVal = -1;
		inputIndex = 0;
		lastInputWhen = lastWhen;
	}

	public void nextFrame(Graphics2D g, FrameInfo frameInfo)
	{

		if (inputIndex >= patternManager.getPatternLength())
		{
			// yay, we beat this level.
			// display last entry for a while before moving on.
			if (frameInfo.getWhen() - lastInputWhen > frameDuration)
			{
				score++;
				game.setAcceptingInput(false);
				return;
			}
		}

		Character input = frameInfo.getUserInput();
		if (input != null)
		{
			lastEnteredVal = Character.getNumericValue(input.charValue());
			if (lastEnteredVal > 2)
				lastEnteredVal--;

			// check if we got the right answer for this index
			if (lastEnteredVal == patternManager.getPatternAtIndex(inputIndex))
			{
				inputIndex++;
			} else
			{
				gameOver = true;
				return;
			}
			lastInputWhen = frameInfo.getWhen();
		}

		// draw the damn thing
		if (lastEnteredVal != -1)
			renderer.render(g, lastEnteredVal);

	}

	public boolean gameOver()
	{
		return gameOver;
	}
	
	public int getScore()
	{
		return score; 
	}
	
}
