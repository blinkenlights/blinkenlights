/*
 * Created by IntelliJ IDEA.
 * User: enno
 * Date: Sep 13, 2002
 * Time: 3:07:03 AM
 * To change template for new class use
 * Code Style | Class Templates options (Tools | IDE Options).
 */
package de.blinkenlights.blinkenlet;

/** This is a class to do the painting. The only actual paint methods are
 * setPixel() and clear(). Painting is (as the name says) buffered, so no
 * visual effect is seen until paint() is called. Usually you don't have to
 * call it yourself, since on return of each method in class Blinkenlet a
 * paint() will occur automatically.
 *
 * Note: DO NOT STORE A REFERENCE TO A BLINKENBUFFER. Do only use the objects
 * passed to you in the methods in which it was passed. If you don't get a
 * buffer-object passed, you are not supposed to draw. That is, draw only in
 * response to calls to BLinkenlet#start, Blinkenlet#stop, Blinkenlet#tick,
 * Blinkenlet#handleEvent and draw only from the thread that called this
 * methods. 
 * 
 */

public class BlinkenBuffer {
    private boolean dirty = true;
    private int width;
    private int height;
    private int channels;
    private Thread paintThread = null;

    int data[];

    BlinkenBuffer(int width, int height, int channels) {
        this.width = width;
        this.height = height;
        this.channels = channels;
        data = new int[width * height * channels];
    }


/* no public ctor */

    /** clear all pixels. */
    public void clear() {
        int k=0;
        for (int i=0; i<width; i++){
            for (int j=0; j < height; j++){
                for (int c = 0; c < channels; c++) {
                    data[k++] = 0;
                }
            }
        }
    };

    /** set first channel of a pixel. since only one channel is supported
     * right now, this just means: set one pixel to a given value. */
    public void setPixel(int x, int y, int value) {
        dirty = true;
        data[(y * width + x) * channels] = value;
    }

    /** flushes this paint buffer and display it. */
    public void paint() {
      if( paintThread != Thread.currentThread() ) {
        System.err.println("warning: painting from wrong thread");
        return;
      }
      paintNative( data );
      dirty = false;
    }


    private native void paintNative(int data[]);

    boolean isDirty() {
        return dirty;
    }

    void startPainting() {
      paintThread = Thread.currentThread();
      dirty = false;
    }

    void finishPainting() {
      if (dirty) {
        paint();
      }
      paintThread = null;
    }
}
