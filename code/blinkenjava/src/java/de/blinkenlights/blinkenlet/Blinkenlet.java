package de.blinkenlights.blinkenlet;

/** This is the main abstract blinkenlet class.  To create an actual blinkenlet
 * extend this class and override at least the methods <tt>prepare</tt> and
 * <tt>start</tt>. You will probably override <tt>tick</tt> and/or
 * <tt>handleEvent</tt>, too, to do s.th. useful. i
 *
 * A general note: all methods except <tt>prepare</tt> are supposed to
 * <i>return quickly</i>. Don't do anything time-consuming in them, don't block
 * on anything. Use timers instead, so you will be called regularly to update
 * the display (use Blinkenlet#startTicker and Blinkenlet#tick for this). 
 *
 * The life of a blinkenlet is the following:
 * <ol>
 * <li> the object will be created. You have to provide a public default
 * constructor (without arguments) or creation will fail. you should not do any
 * lengthy initialization here, as <tt>prepare</tt> is meant for that. don't
 * forget to pass title, author and description to the constructor of
 * Blinkenlet itself.
 * <li> <tt>prepare</tt> will be called. if your blinkenlet needs any
 * initialization/preparation, this is the right place to do it. No visible
 * action is possible here, though, since the blinkenlet is not yet displaying.
 * If you return <tt>true</tt> here, then eventually
 * <li><tt>start</tt> will be called and your blinkenlet is actually running
 * and may draw.  You should now quickly display s.th. so everybody can see it,
 * <li>if anybody calls into blinkenlights with a telephone, you will get
 * BLinkenPlayerEvent's and BlinkenKeyEvent's delivered to
 * Blinkenlet#handleEvent, where you can visually react to them.
 * <li> if you started the ticker using Blinkenlet#startTicker, Blinkenlet#tick
 * will be called after the given timeout. Here you can update the display,
 * e.g. for animations. If you don't want any ticks anymore, use
 * Blinkenlet#stopTicker or return a number less or equal to zero from here.
 * <li>when the system decides to stop the blinkenlet, <tt>stop</tt> will be
 * called.  you can now draw the last time on the display, e.g. a final image,
 * and deinitialize yourself.  but use a finalizer if you have to achieve time
 * consuming tasks.
 *
 * @see Blinkenlet#prepare Blinkenlet#start
 */

abstract public class Blinkenlet {
    static {
        System.loadLibrary("blinkenjava");
    }
    private BlinkenBuffer buffer;
    private String title = "untitled", author="unkown", description="undescribed";

    /**
     * Construct a Blinketlet and provide some information about it.
     * @param title Title of the Blinkenlet.
     * @param author Name(s) of the author(s).
     * @param description A description of the Blinkenlet.
     */
    public Blinkenlet( String title, String author, String description) {
       this.title = title;
       this.author = author;
       this.description = description;
    }


    /** Initialization method for the blinkenlet. The blinkenlet gets passed
     * the width/height of the display, the number of channels and the maximal
     * value to be used in the channels (currently there is always only one channel
     * and the maximal value is always 255 - even if the technology is not capable
     * of actually displaying 256 discrete values, the values you use will be scaled
     * to the actually possible values.)
     * Do all your initialization here! 
     * Return true if you want to be started or false, if not (e.g. because the resolution
     * does not suit you)
     * @param width number of columns of the display
     * @param height number of rows of the display
     * @param channels number of color channels (currently always 1)
     * @param maxval maximal value to be used in the channels (currently fixed to 255)
     * @return true if the blinkenlet wants to be started, false otherwise
     */
    public abstract boolean prepare(int width, int height, int channels, int maxval);

    /** Method to indicate that the blinkenlet is now started. Use the passed BlinkenBuffer
     * to show something on the display to the audience can see you are doing s.th. and/or
     * start a ticker
     * @param buffer a buffer to draw on
     * @see BlinkenBuffer
     * @see Blinkenlet#startTicker
     */
    public abstract void start(BlinkenBuffer buffer);

    /** Method to indicate that the blinkenlet should finish. The last chance to draw 
     * anything before the blinkenlet will actually be stopped. 
     * @param buffer a buffer to draw on
     * @see Blinkenlet#requestStop
     */
    public void stop(BlinkenBuffer buffer) {}

    /**
     * This method will be called if a ticker has been started with
     * Blinkenlet#startTicker after the given timeout. Update the display and
     * return quickly.
     * @param buffer a buffer to draw on
     * @return the time in ms until the next tick should occur or zero if no ticks
     * are wanted anymore.
     * @see Blinkenlet#stopTicker
     */
    public int tick(BlinkenBuffer buffer) { return 0; };

    /**
     * This method will be called ins response to user events, like
     * when a player enters a game or presses a key. Draw on the
     * buffer to show some reaction ...
     * @param event the event that happened (this is either a key event or a
     * player event)
     * @param buffer a buffer to draw on
     * @see BlinkenKeyEvent BlinkenPlayerEvent
     */
    public void handleEvent(BlinkenEvent event, BlinkenBuffer buffer) {};

    /**
     * Start the ticker.
     * @param interval time in ms when the first tick shall occur.
     */
    public native void startTicker( int interval );

    /** Stop the ticker.
     * The blinkenlet will receive no more tick-calls.
     * Use this e.g. if the ticker was turned on to draw an animation
     * but from now on the Blinkenlet will be event-driven only.
     */
    public native void stopTicker();

    /**
     *  Tell the system that this blinkenlet would like to be stopped now,
     *  e.g. game over.
     *  When you are stopped, you will receive a stop call.
     */
    public native void requestStop();

    /** will be called by the system. override this or pass a title to the
     * constructor 
     * @see Blinkenlet#Blinkenlet
     */
    protected String getTitle(){
        return title;
    }

    /** will be called by the system. override this or pass a title to the
     * constructor 
     * @see Blinkenlet#Blinkenlet
     */
    protected String getAuthor() {
        return author;
    }

    /** will be called by the system. override this or pass a title to the
     * constructor 
     * @see Blinkenlet#Blinkenlet
     */
    protected String getDescription() {
        return description;
    }

    /* implementation */
    int tickInternal() {
        /* call tick() */
        buffer.startPainting();
        int timer = tick(buffer);
        buffer.finishPainting();
        return timer;
    }

    boolean prepareInternal(int width, int height, int channels, int maxVal) {
        buffer = new BlinkenBuffer(width, height, channels);
        return prepare(width, height, channels, maxVal);
    }

    void handleEventInternal(BlinkenEvent event) {
        buffer.startPainting();
        handleEvent(event, buffer);
        buffer.finishPainting();
    }

    void startInternal() {
        buffer.startPainting();
        start(buffer);
        buffer.finishPainting();
    }

    void stopInternal() {
	System.out.println("Called stopInternal().");
        buffer.startPainting();
        stop(buffer);
        buffer.finishPainting();
    }
}

