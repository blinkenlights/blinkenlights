#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static int usbio24_fd = -1;

int usbio24_init (const char *devname) {
	int err;
	char c;
	char recv_buf[11];
	
	usbio24_fd = open (devname, O_RDWR);
	if (usbio24_fd < 0) {
		perror ("open");
		return 1;
	}

	/* idetify device */
	c = '?';
	memset (recv_buf, 0, sizeof(recv_buf));
	err = write (usbio24_fd, &c, 1);
	if (err < 0) {
		perror ("write");
		return 2;
	}
	err = read (usbio24_fd, recv_buf, sizeof(recv_buf)-1);
	if (err < sizeof(recv_buf)-1) {
		perror ("read");
		return 3;
	}
	
	printf ("INIT: Device id '%s'\n", recv_buf);

	return 0;
}


int usbio24_set_direction (char port, char direction) {
	char buf[3] = { '!', 'A' + port, direction };
	
	if (write (usbio24_fd, buf, sizeof(buf)) < sizeof(buf)) {
		perror ("write");
		return 1;
	}
	return 0;
}

int usbio24_write (char port, char val) {
	char buf[2] = { 'A' + port, val };

	if (write (usbio24_fd, buf, sizeof(buf)) < sizeof(buf)) {
		perror ("write");
		return 1;
	}
	return 0;
}

char usbio24_read (char port) {
	char buf = 'a' + port;

	if (write (usbio24_fd, &buf, 1) != 1) {
		perror ("write");
		return 0;
	}

	if (read (usbio24_fd, &buf, 1) != 1) {
		perror ("read");
		return 0;
	}
	return buf;
}

