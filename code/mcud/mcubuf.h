/*
 * mcud_buffer.h
 */

#ifndef _BUFFER_H_
#define _BUFFER_H_

/*
 * MCU Protocol buffer types
 */

/*
 * Legacy BLFRAME format
 */

typedef struct bl_frame_body {
	u_int8_t  frame_data[BLFRAME_MAX_HEIGHT][BLFRAME_MAX_WIDTH];
} bl_frame_body_t;

typedef struct bl_frame {
	bl_frame_header_t header;
	bl_frame_body_t body;
} bl_frame_t;


/*
 * MCU format
 */

/*
 * MCU frame body. Note the actual layout of this vector
 * is determined by the height, width and channels parameters
 * within the frame packet, not by the maximum values used here.
 */
typedef struct mcu_frame_body {
	unsigned char data[MCU_MAX_HEIGHT][MCU_MAX_WIDTH][MCU_MAX_CHANNELS];
} mcu_frame_body_t;

typedef struct mcu_frame {
	mcu_frame_header_t header;
	mcu_frame_body_t body;
} mcu_frame_t;


/*
 * MCU setup body
 */

typedef struct mcu_setup_body {
	mcu_setup_pixel_t pixel[MCU_MAX_LAMPS];
} mcu_setup_body_t;

typedef struct mcu_setup {
	mcu_setup_header_t header;
	mcu_setup_body_t body;
} mcu_setup_t;

/*
 * MCU device control body
 */

typedef struct mcu_devctrl_body {
	unsigned char control[MCU_MAX_LAMPS];
} mcu_devctrl_body_t;

typedef struct mcu_devctrl {
	mcu_devctrl_header_t header;
	mcu_devctrl_body_t body;
} mcu_devctrl_t;


#endif /* _BUFFER_H_ */
