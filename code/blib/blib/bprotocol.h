/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2003  The Blinkenlights Crew
 *                          Tim Pritlove <tim@ccc.de>
 *                          Sven Neumann <sven@gimp.org>
 *                          Michael Natterer <mitch@gimp.org>
 *                          Daniel Mack <daniel@yoobay.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __B_PROTOCOL_H__
#define __B_PROTOCOL_H__

#define MAGIC_MCU_SETUP     0x2342FEED  /* MCU Configuration packet              */
#define MAGIC_MCU_FRAME     0x23542666  /* MCU Frame packet                      */
#define MAGIC_MCU_DEVCTRL   0x23542667  /* MCU Device Control packet             */
#define MAGIC_MCU_MULTIFRAME  0x23542668  /* MCU Frame packet                      */

#define MAGIC_BLFRAME       0xDEADBEEF  /* Original BL Frame Packet              */
#define MAGIC_BLFRAME_256   0xFEEDBEEF  /* Extendend BL Frame Packet (Greyscale) */

#define MAGIC_HEARTBEAT     0x42424242  /* Heartbeat packet                      */

#define MCU_LISTENER_PORT    2323
#define MCU_ID_ANY           -1

#define B_HEARTBEAT_PORT       4242
#define B_HEARTBEAT_INTERVAL   5000     /* Heartbeat interval in ms    */

/***********************************************************/

/*
 * Legacy Blinkenlights bl_frame format
 */

typedef struct bl_frame_header bl_frame_header_t;

struct bl_frame_header
{
  guint32 frame_magic;  /* == MAGIC_BLFRAME */
  guint32 frame_count;
  guint16 frame_width;
  guint16 frame_height;
  /*
   * followed by
   * unsigned char data[rows][columns];
   */
};

/***********************************************************/

/*
 * MCU Frame packet
 */

typedef struct mcu_frame_header mcu_frame_header_t;

struct mcu_frame_header
{
  guint32 magic;     /* == MAGIC_MCU_FRAME                        */
  guint16 height;    /* rows                                      */
  guint16 width;     /* columns                                   */
  guint16 channels;  /* Number of channels (mono/grey: 1, rgb: 3) */
  guint16 maxval;    /* maximum pixel value (only 8 bits used)    */
  /*
   * followed by
   * unsigned char data[rows][columns][channels];
   */
};

/*
 * MCU Multi Frame packet
 *
 *
 * - One Multi Frame packet may contain multiple frames, but does not need to
 * - If the number and ids of the subframes vary in consecutive multi frame packets 
 *   then nothing is assumed about the missing subframes. This allows for incremental
 *   updates for only the screens that did change. 
 *
 */

typedef struct mcu_subframe_header mcu_subframe_header_t;

struct mcu_subframe_header
{
  unsigned char screen_id;         /* screen id                                 */
  unsigned char bpp;               /* bits per pixel, supported values: (4,8)   */
  				   /* 4 means nibbles 8 means bytes             */
  guint16 height;                  /* number of rows                            */
  guint16 width;                   /* width in pixels of row                    */
  /*
   * followed by 
   * nibbles in [rows][columns];
   * if width is uneven one nibble is used as padding
   * or bytes[]
   *
   * the bytesize of this can be calculated using height * width in the byte case 
   *   and height * ((width + 1)/2) in case of nibbles (integer divison) 
   */
  guchar data[0];
};

typedef struct mcu_multiframe_header mcu_multiframe_header_t;

struct mcu_multiframe_header
{
  guint32 magic;     /* == MAGIC_MCU_MULTIFRAME                   */
  guint64 timestamp; /* milliseconds since epoch - e.g. gettimeofday(&tv); 
                        timeStamp = tv->tv_sec * 1000 + tv->tv_usec / 1000.; */
  /*
   * followed by multiple subframe headers
   */
  mcu_subframe_header_t subframe[0];
};

/*
 * MCU Setup packet
 */

typedef struct mcu_setup_pixel mcu_setup_pixel_t;

struct mcu_setup_pixel
{
  unsigned char row;
  unsigned char column;
};


typedef struct mcu_setup_header mcu_setup_header_t;

struct mcu_setup_header
{
  guint32     magic;        /* == MAGIC_MCU_SETUP        */

  char          mcu_id;       /* target MCU id ( -1 = any) */
  unsigned char _reserved[3]; /* padding                   */

  guint16     height;
  guint16     width;
  guint16     channels;

  guint16     pixels;       /* number of ports used (starting from 0) */
  /*
   * followed by
   * mcu_setup_pixel_t pixel[pixels];
   */
};


/*
 * MCU Device Control packet
 */

#define MCU_DEVCTRL_COMMAND_SET_LINE	/* set MCU's configured line */

typedef struct mcu_devctrl_header  mcu_devctrl_header_t;

struct mcu_devctrl_header
{
  guint32 magic;        /* == MAGIC_MCU_DEVCTRL                    */
  guint32 command;      /* MCU_DEVCTRL_COMMAND_*                   */
  guint32 value;
  guint32 parameter;
};

/*
 * Heartbeat Packet
 */

typedef struct heartbeat_header heartbeat_header_t;

struct heartbeat_header
{
  guint32 magic;        /* == MAGIC_HEARTBEAT                      */
  guint16 version;      /* hearbeat protocol version number (0)    */
};


#endif /* __B_PROTOCOL_H__ */
