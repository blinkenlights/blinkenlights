/*
 * bmc_pio - BMC PIO card driver for Linux 2.2
 * Copyright (C) 2001 Ingo Albrecht <prom@berlin.ccc.de>
 *
 * Dedicated to the BlinkenLights project.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * This driver allows access to BMC PIO cards based on the 8255 chip.
 * Being a quick hack it is essentially a wonder that this stuff
 * worked for the whole duration of the BlinkenLights project without
 * having to be reloaded or malfunctioning in some other weird way.
 *
 * CAVEATS:
 *
 *  - Cards are not detected because this is simply not possible.
 *  - The driver has some primitive kind of locking, but i am
 *    100% sure this will fail in multiprocessing environments,
 *    and it could even explode when there is more than one process
 *    trying to communicate with the driver. This is absolutely untested.
 *  - This is written for and was only used with 2.2 kernels.
 *  - The interface (although being quite ok compared to other
 *    drivers for these cards) is terrible.
 *  - The driver has always been used with all ports set
 *    to MODE0 and configured as outputs. No other configurations
 *    have ever been tried (at least AFAIK).
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include "bmc_pio.h"

// the number of configured devices
static int bmc_pio_devcount = 0;
// one io address per chip
static u32 bmc_pio_io[BMC_PIO_MAXCHIPS];
// the last control port data written
static u8  bmc_pio_control[BMC_PIO_MAXCHIPS];
// one usage counter per port (3 ports per chip)
static u32 bmc_pio_usage[BMC_PIO_MAXCHIPS*3];
// misc flags - only WRITELOCK at the moment
static u8  bmc_pio_flags[BMC_PIO_MAXCHIPS*3];

MODULE_PARM (bmc_pio_io, "1-" __MODULE_STRING(BMC_PIO_MAXCHIPS) "i");

// forward declarations for our file operations
static int     bmc_pio_open ( struct inode *inode, struct file *file );
static int     bmc_pio_release ( struct inode *inode, struct file *file );
static ssize_t bmc_pio_read ( struct file *file, char *buffer, size_t nbytes, loff_t *ppos );
static ssize_t bmc_pio_write ( struct file *file, const char *buffer, size_t count, loff_t *ppos );
static int     bmc_pio_ioctl ( struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);

// our file operation struct
struct file_operations bmc_pio_fops = {
    open:	bmc_pio_open,
    release:	bmc_pio_release,
    read:	bmc_pio_read,
    write:	bmc_pio_write,
    ioctl:	bmc_pio_ioctl
};

static int bmc_pio_open ( struct inode *inode, struct file *file )
{
  int minor = MINOR (file -> f_dentry -> d_inode -> i_rdev);

  if (minor > bmc_pio_devcount*3 || minor < 0)
    {
#ifdef DEBUG_FOPS
      printk(KERN_NOTICE "bmc_pio: can't open (device %i does not exist)\n", minor );
#endif
      return -ENODEV;
    }

  if ( file -> f_flags & (O_WRONLY | O_RDWR) )
    {
      if ( bmc_pio_usage[BMC_PIO_PORT(minor)] && (bmc_pio_flags[BMC_PIO_PORT(minor)] & BMC_PIO_FLAG_WRLOCK) )
	{
#ifdef DEBUG_FOPS
	  printk(KERN_NOTICE "bmc_pio: can't open (writelock for device %i allready set)\n", minor );
#endif
	  return -EBUSY;
	}
      bmc_pio_flags[BMC_PIO_PORT(minor)] &=  BMC_PIO_FLAG_WRLOCK;
    }

#ifdef DEBUG_FOPS
  printk (KERN_NOTICE "bmc_pio: bmc_pio_open: minor: %i, chip: %i, port: %i", minor, BMC_PIO_CHIP(minor), BMC_PIO_PORT(minor));
#endif

  bmc_pio_usage[BMC_PIO_PORT(minor)]++;
  MOD_INC_USE_COUNT;

  return 0;
}

static int bmc_pio_release ( struct inode *inode, struct file *file )
{
  int minor = MINOR (file -> f_dentry -> d_inode -> i_rdev);

  if ( (file -> f_flags & (O_WRONLY | O_RDWR)))
    bmc_pio_flags[BMC_PIO_PORT(minor)] = bmc_pio_flags[BMC_PIO_PORT(minor)] & (BMC_PIO_FLAG_WRLOCK^1);

#ifdef DEBUG_FOPS
  printk (KERN_NOTICE "bmc_pio: bmc_pio_release: minor: %i, chip: %i, port: %i\n", minor, BMC_PIO_CHIP(minor), BMC_PIO_PORT(minor));
#endif

  bmc_pio_usage[BMC_PIO_PORT(minor)]--;

  MOD_DEC_USE_COUNT;
  return 0;
}

static ssize_t bmc_pio_read ( struct file *file, char * buffer, size_t nbytes, loff_t *ppos )
{
  int minor = MINOR (file -> f_dentry -> d_inode -> i_rdev);
  char tmp;

  if (nbytes != 1)
    return -EINVAL;

  if ((tmp=__put_user(inb(bmc_pio_io[BMC_PIO_CHIP(minor)]+BMC_PIO_PORT(minor)), buffer)))
    return -EFAULT;

#ifdef DEBUG_FOPS
  printk (KERN_NOTICE "bmc_pio: bmc_pio_read: 0x%X, minor: %i, address: 0x%X\n", tmp, minor, bmc_pio_io[BMC_PIO_CHIP(minor)]+BMC_PIO_PORT(minor));
#endif

  return 1;
}

static ssize_t bmc_pio_write ( struct file *file, const char *buffer, size_t count, loff_t *ppos )
{
  int minor = MINOR (file -> f_dentry -> d_inode -> i_rdev);
  char tmp;

  if (count != 1)
    return -EINVAL;

  if (get_user(tmp, (char *) &(buffer[0])))
    return -EFAULT;

#ifdef DEBUG_FOPS
  printk (KERN_NOTICE "bmc_pio: bmc_pio_write: 0x%X, minor: %i, address: 0x%X\n", tmp, minor, bmc_pio_io[BMC_PIO_CHIP(minor)]+BMC_PIO_PORT(minor));
#endif

  outb(tmp, bmc_pio_io[BMC_PIO_CHIP(minor)]+BMC_PIO_PORT(minor));

    return 1;
}

static int bmc_pio_ioctl ( struct inode *inode, struct file *file, unsigned int cmd, unsigned long argp)
{
  int minor = MINOR (file -> f_dentry -> d_inode -> i_rdev);
  char arg;

  if (get_user(arg, (char *)argp))
    return -EFAULT;

#ifdef DEBUG_FOPS
  printk (KERN_NOTICE "bmc_pio: bmc_pio_ioctl: 0x%X, minor: %i, address: 0x%X\n", cmd, minor, bmc_pio_io[BMC_PIO_CHIP(minor)]+BMC_PIO_PORT(minor));
#endif

  switch (cmd)
    {
      // set a port to input/output mode
    case BMC_PIO_CTL_DIRECTION:
#ifdef DEBUG_IOCTLS
      printk(KERN_NOTICE "bmc_pio: ioctl _CTL_DIRECTION minor 0x%x arg 0x%x\n", minor, (unsigned int)arg);
#endif
      switch (BMC_PIO_PORT(minor))
	{
	case 0:
	  if (arg == BMC_PIO_INPUT)
	    bmc_pio_control[BMC_PIO_CHIP(minor)] |= BMC_PIO_PORTA_IN;
	  else if (arg == BMC_PIO_OUTPUT)
	    {
	      bmc_pio_control[BMC_PIO_CHIP(minor)] |= BMC_PIO_PORTA_IN;
	      bmc_pio_control[BMC_PIO_CHIP(minor)] ^= BMC_PIO_PORTA_IN;
	    }
	  else
	    return -EINVAL;
	  break;
	case 1:
	  if (arg == BMC_PIO_INPUT)
	    bmc_pio_control[BMC_PIO_CHIP(minor)] |= BMC_PIO_PORTB_IN;
	  else if (arg == BMC_PIO_OUTPUT)
	    {
	      bmc_pio_control[BMC_PIO_CHIP(minor)] |= BMC_PIO_PORTB_IN;
	      bmc_pio_control[BMC_PIO_CHIP(minor)] ^= BMC_PIO_PORTB_IN;
	    }
	  else
	    return -EINVAL;
	  break;
	case 2:
	  if (arg == BMC_PIO_INPUT)
	    bmc_pio_control[BMC_PIO_CHIP(minor)] |= BMC_PIO_PORTB_IN;
	  else if (arg == BMC_PIO_OUTPUT)
	    {
	      bmc_pio_control[BMC_PIO_CHIP(minor)] |= (BMC_PIO_PORTCU_IN|BMC_PIO_PORTCL_IN);
	      bmc_pio_control[BMC_PIO_CHIP(minor)] ^= (BMC_PIO_PORTCU_IN|BMC_PIO_PORTCL_IN);
	    }
	  else
	    return -EINVAL;
	  break;
	}
      break;
      // set the mode of group A
    case BMC_PIO_CTL_SETMODE_A:
#ifdef DEBUG_IOCTLS
      printk(KERN_NOTICE "bmc_pio: ioctl _CTL_SETMODE_A minor 0x%x arg 0x%x\n", minor, (unsigned int)arg);
#endif
      switch (arg)
	{
	case 0:
	  bmc_pio_control[BMC_PIO_CHIP(minor)] |= BMC_PIO_GROUPA_MALL;
	  bmc_pio_control[BMC_PIO_CHIP(minor)] ^= BMC_PIO_GROUPA_MALL;
	  bmc_pio_control[BMC_PIO_CHIP(minor)] |= BMC_PIO_GROUPA_M0;
	  break;
	case 1:
	  bmc_pio_control[BMC_PIO_CHIP(minor)] |= BMC_PIO_GROUPA_MALL;
	  bmc_pio_control[BMC_PIO_CHIP(minor)] ^= BMC_PIO_GROUPA_MALL;
	  bmc_pio_control[BMC_PIO_CHIP(minor)] |= BMC_PIO_GROUPA_M1;
	  break;
	case 2:
	  bmc_pio_control[BMC_PIO_CHIP(minor)] |= BMC_PIO_GROUPA_MALL;
	  bmc_pio_control[BMC_PIO_CHIP(minor)] ^= BMC_PIO_GROUPA_MALL;
	  bmc_pio_control[BMC_PIO_CHIP(minor)] |= BMC_PIO_GROUPA_M2;
	  break;
	}
      break;
      // set the mode for group B
    case BMC_PIO_CTL_SETMODE_B:
#ifdef DEBUG_IOCTLS
      printk(KERN_NOTICE "bmc_pio: ioctl _CTL_SETMODE_B minor 0x%x arg 0x%x\n", minor, (unsigned int)arg);
#endif
      switch (arg)
	{
	case 0:
	  bmc_pio_control[BMC_PIO_CHIP(minor)] |= BMC_PIO_GROUPB_MALL;
	  bmc_pio_control[BMC_PIO_CHIP(minor)] ^= BMC_PIO_GROUPB_MALL;
	  bmc_pio_control[BMC_PIO_CHIP(minor)] |= BMC_PIO_GROUPB_M0;
	  break;
	case 1:
	  bmc_pio_control[BMC_PIO_CHIP(minor)] |= BMC_PIO_GROUPB_MALL;
	  bmc_pio_control[BMC_PIO_CHIP(minor)] ^= BMC_PIO_GROUPB_MALL;
	  bmc_pio_control[BMC_PIO_CHIP(minor)] |= BMC_PIO_GROUPB_M1;
	  break;
	}
      break;
    default:
      return -EINVAL;
    }

  outb(bmc_pio_control[BMC_PIO_CHIP(minor)], bmc_pio_io[BMC_PIO_CHIP(minor)] + BMC_PIO_PORT_CTL);

  return 0;
}

// reset the device flags, usage counters ...
static void bmc_pio_resetstate ( void )
{
  int a,b;

#ifdef DEBUG_MISC
  printk(KERN_NOTICE "bmc_pio: state reset\n");
#endif

  for (a=0; a<bmc_pio_devcount; a++)
    {
      bmc_pio_control[a] = BMC_PIO_DEFCTL;
      outb(bmc_pio_control[a], bmc_pio_io[a]+BMC_PIO_PORT_CTL);
      for (b=0; b<4; b++)
	{
	  bmc_pio_usage[(a*3)+b] = 0;
	  bmc_pio_flags[(a*3)+b] = BMC_PIO_DEFFLAGS;
	}
    }
}

// module handling stuff

int init_module (void)
{
  int tmp;
  int returned;
  
  if ( !bmc_pio_io[0] )
    {
      printk(KERN_NOTICE "bmc_pio: no io addresses specified\n");
      return -EINVAL;
    }
  
  for (tmp=0; bmc_pio_io[tmp] && tmp < BMC_PIO_MAXCHIPS; tmp++)
    {
      if (bmc_pio_io[tmp]%4)
	{
	  printk(KERN_NOTICE "bmc_pio: io address %i is not on a 4 byte boundary\n", bmc_pio_io[tmp]);
	  return -EINVAL;
	}
      bmc_pio_devcount++;
    }
  
  if ((returned = register_chrdev(BMC_PIO_MAJOR, "BMCPIO", &bmc_pio_fops)))
    {
      printk(KERN_NOTICE "bmc_pio: could not register major %i\n", BMC_PIO_MAJOR);
      return returned;
    }
  
  for (tmp=0; tmp < bmc_pio_devcount; tmp++)
    {
      if((returned = check_region(bmc_pio_io[tmp], 4)))
	{
	  printk(KERN_NOTICE "bmc_pio: 4byte io region @ 0x%x allready in use\n", bmc_pio_io[tmp]);
	  unregister_chrdev(BMC_PIO_MAJOR, "BMCPIO");
	  return returned;
	}
      
      request_region(bmc_pio_io[tmp], 4, "BMCPIO");
    }
  
  bmc_pio_resetstate();
  
  for (tmp=0; tmp < bmc_pio_devcount; tmp++)
    printk(KERN_NOTICE "bmc_pio: 8255 @ 0x%x\n", bmc_pio_io[tmp]);
  
  return 0;
}

void cleanup_module (void)
{
  int tmp;
  
  bmc_pio_resetstate();
  
  for (tmp=0; tmp < bmc_pio_devcount; tmp++)
    release_region(bmc_pio_io[tmp], 4);
  
  unregister_chrdev(BMC_PIO_MAJOR, "BMCPIO");
  
  printk(KERN_NOTICE "bmc_pio: module removed\n");
}
