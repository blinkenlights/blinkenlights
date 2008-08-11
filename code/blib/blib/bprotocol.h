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
  guint16 bpp;       /* bits used per pixel information (only 4 and 8 supported */
  /*
   * followed by
   * unsigned char data[rows * columns * channels * (bpp/8)];
   */
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
  guint16 pixels;       /* number of pixels used (starting from 0) */
  guint16 _reserved;    /* 32 bit padding                          */
  guint32 command;      /* MCU_DEVCTRL_COMMAND_*                   */
  guint32 value;
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
