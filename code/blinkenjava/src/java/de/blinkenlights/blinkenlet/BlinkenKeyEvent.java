/*
 */
package de.blinkenlights.blinkenlet;

/** A User Key event, i.e. user entered a key, usually via mobile phone.
*/
public class BlinkenKeyEvent implements BlinkenEvent{

    private int deviceId;
    private int key;

    public static final int KEY_0 = 0;
    public static final int KEY_1 = 1;
    public static final int KEY_2 = 2;
    public static final int KEY_3 = 3;
    public static final int KEY_4 = 4;
    public static final int KEY_5 = 5;
    public static final int KEY_6 = 6;
    public static final int KEY_7 = 7;
    public static final int KEY_8 = 8;
    public static final int KEY_9 = 9;
    public static final int KEY_HASH  = 10;
    public static final int KEY_ASTERISK = 11;

    BlinkenKeyEvent(int deviceId, int key) {
	this.deviceId = deviceId;
	this.key = key;
    }

    /** The key that was pressed.
     *  Use the constants defined in @see BlinkenEvent for interpretation.
     */
    public int getKey() {
	return key;
    }

    /** The device that caused the event. 
     *  E.g. in multiplayer games, there are sevaral devices.
     */
    public int getDeviceId() {
	return deviceId;
    }

}
