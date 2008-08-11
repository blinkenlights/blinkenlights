/***************************************************************
 *
 * OpenBeacon.org - flash routines for persistent environment storage
 *
 * Copyright 2007 Milosch Meriac <meriac@openbeacon.de>
 *
 ***************************************************************

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef __ENV_H__
#define __ENV_H__

#define TENVIRONMENT_MAGIC 0x23230001

#define AT91C_IFLASH_PAGE_SIZE		((unsigned int) 256)	// Internal FLASH Page Size: 256 bytes
#define AT91C_IFLASH_PAGE_SHIFT		8
#define AT91C_IFLASH_NB_OF_LOCK_BITS	((unsigned int) 16)    // Internal FLASH Number of Lock Bits: 16
#define AT91C_IFLASH_LOCK_REGION_SIZE	(AT91C_IFLASH_SIZE/AT91C_IFLASH_NB_OF_LOCK_BITS)
#define AT91C_MC_CORRECT_KEY		((unsigned int) 0x5A << 24)	// (MC) Correct Protect Key

typedef struct
{
    unsigned int magic,crc16;
    signed int assigned_line;

} TEnvironment;

typedef union {
    TEnvironment e;
    unsigned int data[AT91C_IFLASH_PAGE_SIZE / sizeof(unsigned int)];
} TEnvironmentBlock;

extern void env_store(void) RAMFUNC;
extern int env_load(void);
extern void env_init(void);
extern void env_load_defaults(void);
extern TEnvironmentBlock env;
extern unsigned short env_crc16 (const unsigned char *buffer, int size) RAMFUNC;

#endif/*__ENV_H__*/

