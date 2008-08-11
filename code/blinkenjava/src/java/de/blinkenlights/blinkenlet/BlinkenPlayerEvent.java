/*
 */
package de.blinkenlights.blinkenlet;

/** A Player event, i.e. user entered or left the game. Use the given device id to
 * associate incoming key events with players
*/
public class BlinkenPlayerEvent implements BlinkenEvent{

    /* constants. don't change, because these values are expected
     * on the native side */
    public static final int PLAYER_ENTERED = 0;
    public static final int PLAYER_LEFT    = 1;

    private int type; // left or entered
    private int deviceId;


    BlinkenPlayerEvent(int type, int deviceId) {
      this.deviceId = deviceId;
      this.type = type;
      if( type != PLAYER_ENTERED && type != PLAYER_LEFT) {
        throw new IllegalArgumentException("illegal event type");
      }
    }

    /** The type of event.
     * ß@see PLAYER_ENTERED, PLAYER_LEFT
     */
    public int getType() {
      return type;
    }

    /** The device that caused the event. 
     *  E.g. in multiplayer games, there are sevaral devices.
     */
    public int getDeviceId() {
      return deviceId;
    }

    public String toString() {
      return getClass().getName() + "[type=" + 
        (type == PLAYER_ENTERED ? "PLAYER_ENTERED" : "PLAYER_LEFT") +
        ", deviceId="+ deviceId + "]";
    }
}
