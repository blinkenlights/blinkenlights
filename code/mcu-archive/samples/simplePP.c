/* $Id: simplePP.c,v 1.1.1.1 2002/05/31 22:03:56 tim Exp $ */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <asm/io.h>

int mmival = 21;

#define LPT_PORT 0x378

MODULE_PARM(mmival,"i");
MODULE_LICENSE("GPL");

int ppout( unsigned char byte) {
   outb(byte,LPT_PORT);
   return 1;
}

int init_module(void) {
   printk("simplePP hello mmival = %d\n",mmival);
   ppout((unsigned char ) mmival & 0xff);
   return 0;
}

void cleanup_module(void) {
   printk("simple bye.. mmival = %d\n",mmival);
   ppout((unsigned char ) 0);
   return;
}
