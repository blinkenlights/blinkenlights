#!/usr/bin/php5
/***************************************************************
 *
 * OpenBeacon.org - Gamma Table
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

#ifndef __GAMMATABLE_H__
#define __GAMMATABLE_H__

<?php

define('GAMMA_SIZE' ,100);
define('GAMMA_RANGE',0xFFFF);
define('GAMMA',2);

printf("#define GAMMA       (%d)\n",GAMMA);
printf("#define GAMMA_SIZE  (%d)\n",GAMMA_SIZE);
printf("#define GAMMA_RANGE (0x%04X)\n\n",GAMMA_RANGE);

echo 'static unsigned short GammaTable[GAMMA_SIZE]={';

for($i=0; $i<GAMMA_SIZE; $i++)
{
    $ts = round((GAMMA_RANGE*acos(2*pow($i/GAMMA_SIZE,GAMMA)-1))/M_PI);
        
    if(($i%5)==0)
	printf("\n\t");
	
    printf("0x%04X,",$ts);
}
?>};


#endif/*__GAMMATABLE_H__*/
