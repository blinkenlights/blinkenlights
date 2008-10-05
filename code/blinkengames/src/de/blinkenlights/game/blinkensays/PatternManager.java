package de.blinkenlights.game.blinkensays;

import java.awt.Graphics2D;

import de.blinkenlights.game.FrameInfo;

public class PatternManager {
    SimonRenderer renderer;
    BlinkenSays game;

    int patternLength;
    int[] pattern;
    int playbackIndex = 0;

    long lastPlaybackWhen = 0;
    long frameDuration = 1000;
    boolean delay;

    public PatternManager(BlinkenSays game, SimonRenderer renderer) {
        this.game = game;
        this.renderer = renderer;
        patternLength = 2;
        delay = true;

        pattern = new int[patternLength];
        int last = -1;
        for (int i = 0; i < patternLength; i++) {
            int rand = 1 + (int) (Math.random() * 4);

            // don't allow repeated numbers (for now)
            while (rand == last)
                rand = 1 + (int) (Math.random() * 4);
            pattern[i] = rand;
            last = pattern[i];
        }
    }

    public void prepareNextPattern(long lastWhen) {
        int[] oldPattern = new int[patternLength];
        for (int i = 0; i < oldPattern.length; i++)
            oldPattern[i] = pattern[i];

        patternLength++;

        pattern = new int[patternLength];

        // copy old elements into new array
        for (int i = 0; i < oldPattern.length; i++)
            pattern[i] = oldPattern[i];

        // add new element
        int rand = 1 + (int) (Math.random() * 4);
        int prev = oldPattern[oldPattern.length - 1];

        while (rand == prev)
            rand = 1 + (int) (Math.random() * 4);

        pattern[patternLength - 1] = rand;

        playbackIndex = 0;
        lastPlaybackWhen = lastWhen;
        delay = true;
    }

    public void nextFrame(Graphics2D g, FrameInfo frameInfo) {
        long now = frameInfo.getWhen();

        // pass control to the input listener if we are done playing back
        if (playbackIndex >= patternLength) {
            game.setAcceptingInput(true);
            return;
        }

        // delay before starting the animation
        if (delay && now - lastPlaybackWhen < frameDuration) {
            return;
        } else if (delay) {
            delay = false;
            lastPlaybackWhen = now;
            return;
        }

        // draw the current value
        renderer.render(g, getPatternAtIndex(playbackIndex));

        // increment the animation?
        if (now - lastPlaybackWhen > frameDuration) {
            playbackIndex++;
            lastPlaybackWhen = now;
        }
    }

    public int getPatternLength() {
        return patternLength;
    }

    public int getPatternAtIndex(int index) {
        return pattern[index];
    }
}
