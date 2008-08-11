/*
	FreeRTOS.org V5.0.0 - Copyright (C) 2003-2008 Richard Barry.

	This file is part of the FreeRTOS.org distribution.

	FreeRTOS.org is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	FreeRTOS.org is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with FreeRTOS.org; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	A special exception to the GPL can be applied should you wish to distribute
	a combined work that includes FreeRTOS.org, without being obliged to provide
	the source code for any proprietary components.  See the licensing section
	of http://www.FreeRTOS.org for full details of how and when the exception
	can be applied.
*/

/* Library includes. */
#include <string.h>
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* application includes. */
#include "beacontypes.h"
#include "network.h"
#include "USB-CDC.h"
#include "bprotocol.h"
#include "env.h"
#include "led.h"
#include "usb.h"
#include "proto.h"

/* lwIP includes. */
#include "lwip/api.h"

/* Hardware specific headers. */
#include "board.h"
#include "AT91SAM7X256.h"

/* Priorities/stacks for the various tasks within the demo application. */
#define mainQUEUE_POLL_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define mainCHECK_TASK_PRIORITY		( tskIDLE_PRIORITY + 3 )
#define mainSEM_TEST_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define mainFLASH_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define mainBLOCK_Q_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define mainNETWORK_PRIORITY            ( tskIDLE_PRIORITY + 2 )
#define mainUSB_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define mainUSB_TASK_STACK		( 200 )

static void
prvSetupHardware (void)
{
  /* When using the JTAG debugger the hardware is not always initialised to
     the correct default state.  This line just ensures that this does not
     cause all interrupts to be masked at the start. */
  AT91C_BASE_AIC->AIC_EOICR = 0;
  
  vLedInit();

  /* Enable the peripheral clock. */
  AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_PIOA;
  AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_PIOB;
  AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_EMAC;
}

/*
 * Setup hardware then start all the demo application tasks.
 */
int
main (void)
{
  prvSetupHardware ();

  vlwIPInit ();

  /* Create the lwIP task.  This uses the lwIP RTOS abstraction layer. */
  sys_thread_new (networkThread, (void *) NULL, mainNETWORK_PRIORITY);

  /* Create the demo USB CDC task. */
  xTaskCreate (vUSBCDCTask, (signed portCHAR *) "USB", mainUSB_TASK_STACK,
	       NULL, mainUSB_PRIORITY, NULL);

  env_init();
  if (!env_load())
    env_load_defaults();
        
  vInitProtocolLayer ();

  /* Finally, start the scheduler. 

     NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
     The processor MUST be in supervisor mode when vTaskStartScheduler is 
     called.  The demo applications included in the FreeRTOS.org download switch
     to supervisor mode prior to main being called.  If you are not using one of
     these demo application projects then ensure Supervisor mode is used here. */
  vTaskStartScheduler ();

  /* Should never get here! */
  return 0;
}

/*-----------------------------------------------------------*/

void vApplicationIdleHook(void)
{
}

