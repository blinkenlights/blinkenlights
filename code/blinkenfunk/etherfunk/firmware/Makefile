#	FreeRTOS.org V4.2.1 - Copyright (C) 2003-2007 Richard Barry.
#
#	This file is part of the FreeRTOS.org distribution.
#
#	FreeRTOS.org is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	FreeRTOS.org is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with FreeRTOS.org; if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#	A special exception to the GPL can be applied should you wish to distribute
#	a combined work that includes FreeRTOS.org, without being obliged to provide
#	the source code for any proprietary components.  See the licensing section
#	of http://www.FreeRTOS.org for full details of how and when the exception
#	can be applied.
#
#	***************************************************************************
#	See http://www.FreeRTOS.org for documentation, latest information, license
#	and contact details.  Please ensure to read the configuration and relevant
#	port sections of the online documentation.
#	***************************************************************************

CC=arm-elf-gcc
OBJCOPY=arm-elf-objcopy
OBJDUMP=arm-elf-objdump
ARCH=arm-elf-ar
CRT0=os/boot/boot.s
DEBUG=-g
OPTIM=-Os
LDSCRIPT=config/atmel-rom.ld
ARCH=AT91SAM7S128
TARGETNAME=etherfunk

#
# CFLAGS common to both the THUMB and ARM mode builds
#

CFLAGS= \
-D __$(ARCH)__ \
-Iconfig \
-Iapplication \
-Iapplication/nRF24L01 \
-Ios/core/include  \
-Ios/usb  \
-Ios/core/ARM7_AT91SAM7S  \
-Ios/lwip-1.1.0/src/include \
-Ios/lwip-1.1.0/src/include/ipv4 \
-Ios/lwip-1.1.0/contrib/port/FreeRTOS/AT91SAM7X/ \
-Ios/core/ARM7_AT91SAM7S/EMAC \
-Wall  \
-Werror \
-Wextra  \
-Wno-multichar \
-Wstrict-prototypes  \
-Wno-strict-aliasing  \
-Wno-unused-parameter \
-D SAM7_GCC  \
-mcpu=arm7tdmi  \
-ffunction-sections \
-fdata-sections \
-T$(LDSCRIPT) \
$(DEBUG)  \
$(OPTIM) \
-fomit-frame-pointer

LINKER_FLAGS=-Xlinker -o${TARGETNAME}.elf -Xlinker -M -Xlinker -Map=${TARGETNAME}.map

#
# Source files that must be built to ARM mode.
#

LWIP_SRC= \
  os/lwip-1.1.0/src/core/tcp_out.c \
  os/lwip-1.1.0/src/core/inet.c \
  os/lwip-1.1.0/src/core/mem.c \
  os/lwip-1.1.0/src/core/memp.c \
  os/lwip-1.1.0/src/core/netif.c \
  os/lwip-1.1.0/src/core/dhcp.c \
  os/lwip-1.1.0/src/core/pbuf.c \
  os/lwip-1.1.0/src/core/raw.c \
  os/lwip-1.1.0/src/core/stats.c \
  os/lwip-1.1.0/src/core/sys.c \
  os/lwip-1.1.0/src/core/tcp.c \
  os/lwip-1.1.0/src/core/tcp_in.c \
  os/lwip-1.1.0/src/core/ipv4/ip.c \
  os/lwip-1.1.0/src/core/ipv4/ip_addr.c \
  os/lwip-1.1.0/src/core/ipv4/icmp.c \
  os/lwip-1.1.0/src/api/tcpip.c \
  os/lwip-1.1.0/src/api/api_msg.c \
  os/lwip-1.1.0/src/api/err.c \
  os/lwip-1.1.0/src/api/api_lib.c \
  os/lwip-1.1.0/src/netif/etharp.c \
  os/lwip-1.1.0/contrib/port/FreeRTOS/AT91SAM7X/sys_arch.c \
  os/lwip-1.1.0/src/netif/ethernetif.c \
  os/core/ARM7_AT91SAM7S/EMAC/SAM7_EMAC.c \
  os/core/ARM7_AT91SAM7S/EMAC/SAM7_EMAC_ISR.c \
  os/lwip-1.1.0/src/core/udp.c \
  os/lwip-1.1.0/src/core/ipv4/ip_frag.c

ARM_SRC= \
  application/main.c \
  application/network.c \
  application/bprotocol.c \
  application/usb.c \
  application/led.c \
  application/proto.c \
  application/xxtea.c \
  application/env.c \
  application/nRF24L01/nRF_API.c \
  application/nRF24L01/nRF_CMD.c \
  os/boot/Cstartup_SAM7.c \
  os/core/list.c \
  os/core/queue.c \
  os/core/tasks.c \
  os/core/ARM7_AT91SAM7S/port.c \
  os/core/ARM7_AT91SAM7S/portISR.c \
  os/core/MemMang/heap_2.c \
  os/usb/USB-CDC.c \
  os/usb/USBIsr.c \
  $(LWIP_SRC)

#
# Define all object files.
#
ARM_OBJ = $(ARM_SRC:.c=.o)
FREERTOS_THUMB_OBJ = $(FREERTOS_THUMB_SRC:.c=.o)
DEMO_APP_THMUB_OBJ = $(DEMO_APP_THMUB_SRC:.c=.o)

etherfunk.bin : etherfunk.elf
	$(OBJCOPY) etherfunk.elf -O binary etherfunk.bin

etherfunk.hex : etherfunk.elf
	$(OBJCOPY) etherfunk.elf -O ihex etherfunk.hex

etherfunk.elf : $(ARM_OBJ) $(DEMO_APP_THMUB_OBJ) $(FREERTOS_THUMB_OBJ) $(CRT0) Makefile config/FreeRTOSConfig.h
	$(CC) $(CFLAGS) $(ARM_OBJ) $(DEMO_APP_THMUB_OBJ) $(FREERTOS_THUMB_OBJ) -nostartfiles $(CRT0) $(LINKER_FLAGS)
	$(OBJDUMP) -d etherfunk.elf > etherfunk.asm

$(DEMO_APP_THMUB_OBJ)  : %.o : %.c $(LDSCRIPT) Makefile config/FreeRTOSConfig.h
	$(CC) -c $(THUMB_FLAGS) $(CFLAGS) $< -o $@

$(FREERTOS_THUMB_OBJ)  : %.o : %.c $(LDSCRIPT) Makefile config/FreeRTOSConfig.h
	$(CC) -c $(THUMB_FLAGS) $(CFLAGS) $< -o $@

$(ARM_OBJ) : %.o : %.c $(LDSCRIPT) Makefile config/FreeRTOSConfig.h
	$(CC) -c $(CFLAGS) $< -o $@

clean :
	touch Makefile
	find -name '*.o' -exec rm \{\} \;
	rm -f ${TARGETNAME}.bin ${TARGETNAME}.elf ${TARGETNAME}.map ${TARGETNAME}.asm

