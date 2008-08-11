int usbio24_init (const char *devname);
int usbio24_set_direction (char port, char direction);
int usbio24_write (char port, char val);
char usbio24_read (char port);

