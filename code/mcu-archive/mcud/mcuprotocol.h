/*
 * mcuprotocol.h
 */

#ifndef _MCUPROTOCOL_H_
#define _MCUPROTOCOL_H_

#define MAGIC_MCU_SETUP     0x2342FEED    // MCU Configuration packet
#define MAGIC_MCU_FRAME     0x23542666    // MCU Frame packet
#define MAGIC_MCU_DEVCTRL   0x23542667    // MCU Device Control packet

#define MAGIC_BLFRAME       0xDEADBEEF    // Original BL Frame Packet
#define MAGIC_BLFRAME_256   0xFEEDBEEF    // Extendend BL Frame Packet (Greyscale)


#define MCU_LISTENER_PORT    2323

#define MCU_MAX_PIXELS  576               // 24 * 24

#define MCU_MAX_HEIGHT 32
#define MCU_MAX_WIDTH  32

#define MCU_DEPTH_MONOCHROME 1
#define MCU_DEPTH_GREYSCALE  1
#define MCU_DEPTH_RGB        3

#define MCU_MAX_DEPTH        MCU_DEPTH_RGB

#define MCU_ID_ANY           -1

/***********************************************************/

#define BLFRAME_MAX_HEIGHT  8
#define BLFRAME_MAX_WIDTH   18

/*
 * Legacy Blinkenlights bl_frame format
 */

typedef struct bl_frame_header {
	u_int32_t frame_magic;	// == MAGIC_BLFRAME
	u_int32_t frame_count;
	u_int16_t frame_width;
	u_int16_t frame_height;
} bl_frame_header_t;

typedef struct bl_frame_body {
	u_int8_t  frame_data[BLFRAME_MAX_HEIGHT][BLFRAME_MAX_WIDTH];
} bl_frame_body_t;

typedef struct bl_frame {
	bl_frame_header_t header;
	bl_frame_body_t body;
} bl_frame_t, BL_FRAME;

/***********************************************************/

/*
 * MCU Frame packet
 */
typedef struct mcu_frame_header {
	u_int32_t magic;			// == MAGIC_MCU_FRAME
	u_int16_t height;			// rows
	u_int16_t width;			// columns
	u_int16_t depth;			// Values per Pixel (mono/grey: 1, rgb: 3)
 	u_int16_t maxval;		    // maximum pixel value (only 8 bits used)
	/*
	 * followed by
     * unsigned char data[rows][columns][depth];
     */
} mcu_frame_header_t;

/*
 * Frame body. Note the actual layout of this vector
 * is determined by the height, width and depth parameters
 * within the frame packet, not by the maximum values used here.
 */
typedef struct mcu_frame_body {
	unsigned char data[MCU_MAX_HEIGHT][MCU_MAX_WIDTH][MCU_MAX_DEPTH];
} mcu_frame_body_t;

typedef struct mcu_frame {
	mcu_frame_header_t header;
	mcu_frame_body_t body;
} mcu_frame_t, MCU_FRAME;



/*
 * MCU Setup packet
 */

typedef struct mcu_setup_header {
	u_int32_t magic;			// == MAGIC_MCU_SETUP

	char  mcu_id;				// target MCU id ( -1 = any)
	unsigned char  _reserved[3];		// padding

	u_int16_t height;
	u_int16_t width;
	u_int16_t depth;

	u_int16_t pixels;			// number of ports used (starting from 0)
	/*
	 * followed by
     * mcu_setup_pixel_t pixel[pixels];
     */
} mcu_setup_header_t;

typedef struct mcu_setup_pixel {
	unsigned char row;
	unsigned char column;
} mcu_setup_pixel_t;

typedef struct mcu_setup_body {
	mcu_setup_pixel_t pixel[MCU_MAX_PIXELS];
} mcu_setup_body_t;

typedef struct mcu_setup {
	mcu_setup_header_t header;
	mcu_setup_body_t body;
} mcu_setup_t, MCU_SETUP;



/*
 * MCU Device Control packet
 */

#define MCU_DEVCTRL_OFF          0    // switch device off
#define MCU_DEVCTRL_ON           1    // switch device on

typedef struct mcu_devctrl_header {
	u_int32_t magic;			// == MAGIC_MCU_DEVCTRL
	u_int16_t pixels;			// number of pixels used (starting from 0)
	u_int16_t _reserved;		// 32 bit padding

	/*
	 * followed by
     * unsigned char control[ports];
     */
} mcu_devctrl_header_t;

typedef struct mcu_devctrl_body {
	unsigned char control[MCU_MAX_PIXELS];
} mcu_devctrl_body_t;

typedef struct mcu_devctrl {
	mcu_devctrl_header_t header;
	mcu_devctrl_body_t body;
} mcu_devctrl_t, MCU_DEVCTRL;


#endif /* _MCUPROTOCOL_H_ */
