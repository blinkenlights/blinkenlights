/*
 * mcud.h
 */

#ifndef _MCUDEV_H_
#define _MCUDEV_H_

/*
 * MCU Ports Kernel Interface
 *
 * A mcu_device structure is the link between mcud
 * and the device driver in kernel mode. The mcu_device
 * structure resides in shared memory and is
 * initialized by the device driver with the appropriate
 * device type and operation mode.
 */

#define MCU_DEVICE_SHMEM_MAGIC 0x23234242

typedef struct mcu_device {
	unsigned char name[32];			// driver name
	u_int16_t flag_loaded;		// Flag indicating loaded driver
 	u_int16_t flag_zero_off;	// Flag indicating if 0 switches of port
	u_int16_t channels;			// Number of channels (mono/grey: 1, rgb: 3)
 	u_int16_t maxval;		    // maximum pixel value
	unsigned char pixel[MCU_MAX_LAMPS][MCU_MAX_CHANNELS];
} mcu_device_t;


#endif /* _MCUDEV_H_ */
