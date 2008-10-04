package de.blinkenlights.game.blinkensays;

import java.awt.Graphics2D;

import de.blinkenlights.game.FrameInfo;

public class PatternManager
{
	SimonRenderer renderer;
	BlinkenSays game;

	int patternLength;
	int[] pattern;
	int playbackIndex = 0;
	
    long lastPlaybackWhen = 0;
    long frameDuration = 1000;
    boolean delay;
    
	public PatternManager(BlinkenSays game, SimonRenderer renderer)
	{
		this.game = game;
		this.renderer = renderer;
		patternLength = 2;
		delay = true;
	}
	
	public void prepareNextPattern(long lastWhen)
	{
		patternLength++;
		
		pattern = new int[patternLength];
		
		System.out.print("new pattern: ");
		
		// populate the pattern
    	int last = -1;
    	for (int i=0; i<patternLength; i++)
    	{
    		int rand = 1+(int) (Math.random() * 4);
    		
    		// don't allow repeated numbers (for now)
    		while(rand == last)
    			rand = 1 + (int) (Math.random() * 4);
    		pattern[i] = rand;
    		last = pattern[i];
    		
    		if (last > 2)
    			System.out.print(last+1 + " ");
    		else
    			System.out.print(last + " ");
    	}
    	System.out.println();
    	
    	playbackIndex = 0;
    	lastPlaybackWhen = lastWhen;
    	delay = true;
	}
	
	public void nextFrame(Graphics2D g, FrameInfo frameInfo)
	{
		long now = frameInfo.getWhen();
		
		// pass control to the input listener if we are done playing back
		if (playbackIndex >= patternLength)
		{
			game.setAcceptingInput(true);
			return;
		}
				
		// delay before starting the animation
		if (delay && now - lastPlaybackWhen < frameDuration)
		{
			return;
		}
		else if (delay)
		{
			delay = false;
			lastPlaybackWhen = now;
			return;
		}
		
		
		// draw the current value
		renderer.render(g, getPatternAtIndex(playbackIndex));
		
		// increment the animation?
		if (now-lastPlaybackWhen > frameDuration)
		{	
			playbackIndex++;	
			lastPlaybackWhen = now;
		}
	}
	
	public int getPatternLength()
	{
		return patternLength;
	}
	
	public int getPatternAtIndex(int index)
	{
		return pattern[index];
	}
}
