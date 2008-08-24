package de.blinkenlights.bmix;

import java.util.Properties;

public class Version {

    /**
     * The version number of BMix in this classloader context.
     */
    public static final String VERSION;
    static {
        try {
            Properties props = new Properties();
            props.load(Version.class.getResourceAsStream("version.properties"));
            VERSION = props.getProperty("bmix.version");
        } catch (Exception e) {
            throw new RuntimeException("Failed to read version from classpath resource", e);
        }
    }
}
