/*----------------------------------------------------------------------------
*         ATMEL Microcontroller Software Support  -  ROUSSET  -
*----------------------------------------------------------------------------
* The software is delivered "AS IS" without warranty or condition of any
* kind, either express, implied or statutory. This includes without
* limitation any warranty or condition with respect to merchantability or
* fitness for any particular purpose, or against the infringements of
* intellectual property rights of others.
*----------------------------------------------------------------------------
* File Name           : Board.h
* Object              : AT91SAM7X Evaluation Board Features Definition File.
*
* Creation            : JG   20/Jun/2005
*----------------------------------------------------------------------------
*/
#ifndef Board_h
#define Board_h

#include "AT91SAM7X256.h"

#define true	-1
#define false	0

/*-------------------------------*/
/* SAM7Board Memories Definition */
/*-------------------------------*/
// The AT91SAM7X128 embeds a 32-Kbyte SRAM bank, and 128K-Byte Flash

#define  ENVIRONMENT_SIZE       1024
#define  FLASH_PAGE_NB		256
#define  FLASH_PAGE_SIZE	128


/*-------------------------------*/
/* LED declaration               */
/*-------------------------------*/

#define LED_GREEN       (1L<<27)
#define LED_RED         (1L<<28)
#define LED_MASK        (LED_GREEN|LED_RED)

/*-------------------------------*/
/* nRF declaration               */
/*-------------------------------*/

#define DEFAULT_CHANNEL 81

#define CSN_PIN         (1L<<12)
#define MISO_PIN        (1L<<16)
#define MOSI_PIN        (1L<<17)
#define SCK_PIN         (1L<<18)
#define IRQ_PIN         (1L<<14)
#define CE_PIN          (1L<<29)

/*-------------------------*/
/* Push Buttons Definition */
/*-------------------------*/

#define SW1_MASK        (1<<29)	// PB29
#define SW_MASK         (SW1_MASK)
#define SW1 		(1<<29)	// PB29

/*--------------*/
/* Master Clock */
/*--------------*/

#define EXT_OC          18432000	// Exetrnal ocilator MAINCK
#define MCK             47923200	// MCK (PLLRC div by 2)
#define MCKKHz          (MCK/1000)	//

/*----------------*/
/* Crpto Settings */
/*----------------*/

#define CONFIG_TEA_ENABLEDECODE

/*----------------*/
/* Task Settings  */
/*----------------*/

#define TASK_NRF_PRIORITY       ( tskIDLE_PRIORITY + 3 )
#define TASK_NRF_STACK          ( 512 )



#endif /* Board_h */
