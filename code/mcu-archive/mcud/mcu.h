/* mcu.h
 *
 * utility routines and definitions for MCU Devices
 */

#ifndef _MCU_H_
#define _MCU_H_


#define TRUE            1
#define FALSE           0

extern void csc_write (unsigned char index, unsigned char data);
extern unsigned char csc_read (unsigned char index);

extern void mcu_setup_pio(void);
extern void mcu_set_io_ports(unsigned char value);

/*******************************************************************
 * 8255 PIO CHIPS
 */

#define MCU_PORTS       6

extern int pio_port_io_addr[MCU_PORTS];

/*
 * define MCU specific IO addresses
 */

#define PIO_A_BASE     0x0200
#define PIO_B_BASE     0x0280

#define PIO_PORT_A     0x00
#define PIO_PORT_B     0x01
#define PIO_PORT_C     0x02
#define PIO_CONTROL    0x03

/*******************************************************************
 * define AMD Elan SC 410 specific register names
 */

#define CSCIR          0x22    // SC410 CSC Index Register
#define CSCDP          0x23    // SC410 CSC Data Port


#define GPIO_TCR_A     0x3B    // GPIO Termination Control Register A

#define CS7_PUEN       0x80
#define CS6_PUEN       0x40
#define CS5_PUEN       0x20
#define CS4_PUEN       0x10
#define CS3_PUEN       0x08
#define CS2_PUEN       0x04
#define CS1_PUEN       0x02
#define CS0_PUEN       0x01


#define GPIO_TCR_B     0x3C    // GPIO Termination Control Register B
#define GPIO_TCR_C     0x3D    // GPIO Termination Control Register C
#define GPIO_TCR_D     0x3E    // GPIO Termination Control Register D


#define PMU_FMR        0x40    // PMU Force Mode Register

#define LS_TIMER_CNT   0x80   // Low-Speed Timer Count Reset
#define EN_HYPER       0x40   // Enable Hyper-Speed Mode
#define EN_SB_LCD      0x20   // Enable Graphics Operation in Standby Mode
#define FAST_TIMEO     0x10   // Speed Up Suspend and Standby Mode Timers for PMU Code Debug
#define HS_COUNTING    0x08   // High-Speed Clock Delay Timer Status
#define PMU_FORCE      0x07   // Force Mode of PMU
#define PMU_FORCE2     0x04
#define PMU_FORCE1     0x02
#define PMU_FORCE0     0x01


#define CPU_CSR        0x80    // CPU Clock Speed Register

#define CPUCLK_PRES_SP  0xE0   // Present Speed of the CPU Clock
#define CPUCLK_PRES_SP2 0x80
#define CPUCLK_PRES_SP1 0x40
#define CPUCLK_PRES_SP0 0x20
#define LS_CPUCLK      0x18    // CPU Clock Speed in Low-Speed Mode
#define LS_CPUCLK1     0x10
#define LS_CPUCLK0     0x08
#define HS_CPUCLK      0x06    // CPU Clock Speed in High-Speed Mode
#define HS_CPUCLK1     0x04
#define HS_CPUCLK0     0x02
#define HYS_CPUCLK     0x01    // CPU Clock Speed in Hyper-Speed Mode

#define GPIO_CS_FSR_A  0xA0    // GPIO_CS Function Select Register A

#define CS3_PRI        0x80    // GPIO_CS3 Signal is a Primary Activity and Wake-up (Falling Edge) Enable
#define CS3_DIR        0x40    // GPIO_CS3 Signal is an Input/Output
#define CS2_PRI        0x20    // GPIO_CS2 Signal is a Primary Activity and Wake-up (Falling Edge) Enable
#define CS2_DIR        0x10    // GPIO_CS2 Signal is an Input/Output
#define CS1_PRI        0x08    // GPIO_CS1 Signal is a Primary Activity and Wake-up (Falling Edge) Enable
#define CS1_DIR        0x04    // GPIO_CS1 Signal is an Input/Output
#define CS0_PRI        0x02    // GPIO_CS0 Signal is a Primary Activity and Wake-up (Falling Edge) Enable
#define CS0_DIR        0x01    // GPIO_CS0 Signal is an Input/Output


#define GPIO_CS_FSR_B  0xA1    // GPIO_CS Function Select Register B

#define CS7_PRI        0x80    // GPIO_CS7 Signal is a Primary Activity and Wake-up (Falling Edge) Enable
#define CS7_DIR        0x40    // GPIO_CS7 Signal is an Input/Output
#define CS6_PRI        0x20    // GPIO_CS6 Signal is a Primary Activity and Wake-up (Falling Edge) Enable
#define CS6_DIR        0x10    // GPIO_CS6 Signal is an Input/Output
#define CS5_PRI        0x08    // GPIO_CS5 Signal is a Primary Activity and Wake-up (Falling Edge) Enable
#define CS5_DIR        0x04    // GPIO_CS5 Signal is an Input/Output
#define CS4_PRI        0x02    // GPIO_CS4 Signal is a Primary Activity and Wake-up (Falling Edge) Enable
#define CS4_DIR        0x01    // GPIO_CS4 Signal is an Input/Output


#define GPIO_CS_FSR_C  0xA2    // GPIO_CS Function Select Register C

#define CS11_PRI       0x80    // GPIO_CS11 Signal is a Primary Activity and Wake-up (Falling Edge) Enable
#define CS11_DIR       0x40    // GPIO_CS11 Signal is an Input/Output
#define CS10_PRI       0x20    // GPIO_CS10 Signal is a Primary Activity and Wake-up (Falling Edge) Enable
#define CS10_DIR       0x10    // GPIO_CS10 Signal is an Input/Output
#define CS9_PRI        0x08    // GPIO_CS9 Signal is a Primary Activity and Wake-up (Falling Edge) Enable
#define CS9_DIR        0x04    // GPIO_CS9 Signal is an Input/Output
#define CS8_PRI        0x02    // GPIO_CS8 Signal is a Primary Activity and Wake-up (Falling Edge) Enable
#define CS8_DIR        0x01    // GPIO_CS8 Signal is an Input/Output


#define GPIO_CS_FSR_D  0xA3    // GPIO_CS Function Select Register D

#define GPIO15_DIR     0x40    // GPIO15 Signal is an Input/Output
#define CS14_PRI       0x20    // GPIO_CS14 Signal is a Primary Activity and Wake-up (Falling Edge) Enable
#define CS14_DIR       0x10    // GPIO_CS14 Signal is an Input/Output
#define CS13_PRI       0x08    // GPIO_CS13 Signal is a Primary Activity and Wake-up (Falling Edge) Enable
#define CS13_DIR       0x04    // GPIO_CS13 Signal is an Input/Output
#define CS12_PRI       0x02    // GPIO_CS12 Signal is a Primary Activity and Wake-up (Falling Edge) Enable
#define CS12_DIR       0x01    // GPIO_CS12 Signal is an Input/Output


#define GPIO_FSR_E     0xA4    // GPIO Function Select Register E

#define GPIO23_DIR     0x80    // GPIO23 Signal is an Input/Output
#define GPIO22_DIR     0x40    // GPIO22 Signal is an Input/Output
#define GPIO21_DIR     0x20    // GPIO21 Signal is an Input/Output
#define GPIO20_DIR     0x10    // GPIO20 Signal is an Input/Output
#define GPIO19_DIR     0x08    // GPIO19 Signal is an Input/Output
#define GPIO18_DIR     0x04    // GPIO18 Signal is an Input/Output
#define GPIO17_DIR     0x02    // GPIO17 Signal is an Input/Output
#define GPIO16_DIR     0x01    // GPIO16 Signal is an Input/Output


#define GPIO_FSR_F     0xA5    // GPIO Function Select Register F

#define GPIO31_DIR     0x80    // GPIO23 Signal is an Input/Output
#define GPIO30_DIR     0x40    // GPIO22 Signal is an Input/Output
#define GPIO29_DIR     0x20    // GPIO21 Signal is an Input/Output
#define GPIO28_DIR     0x10    // GPIO20 Signal is an Input/Output
#define GPIO27_DIR     0x08    // GPIO19 Signal is an Input/Output
#define GPIO26_DIR     0x04    // GPIO18 Signal is an Input/Output
#define GPIO25_DIR     0x02    // GPIO17 Signal is an Input/Output
#define GPIO24_DIR     0x01    // GPIO16 Signal is an Input/Output


#define GPIO_RBWR_A    0xA6    // GPIO Read-Back/Write Register A

#define GP7STAT_CTL    0x80
#define GP6STAT_CTL    0x40
#define GP5STAT_CTL    0x20
#define GP4STAT_CTL    0x10
#define GP3STAT_CTL    0x08
#define GP2STAT_CTL    0x04
#define GP1STAT_CTL    0x02
#define GP0STAT_CTL    0x01


#define GPIO_RBWR_B    0xA7    // GPIO Read-Back/Write Register B

#define GP15STAT_CTL   0x80
#define GP14STAT_CTL   0x40
#define GP13STAT_CTL   0x20
#define GP12STAT_CTL   0x10
#define GP11STAT_CTL   0x08
#define GP10STAT_CTL   0x04
#define GP9STAT_CTL    0x02
#define GP8STAT_CTL    0x01


#define GPIO_RBWR_C    0xA8    // GPIO Read-Back/Write Register C

#define GP23STAT_CTL   0x80
#define GP22STAT_CTL   0x40
#define GP21STAT_CTL   0x20
#define GP20STAT_CTL   0x10
#define GP19STAT_CTL   0x08
#define GP18STAT_CTL   0x04
#define GP17STAT_CTL   0x02
#define GP16STAT_CTL   0x01


#define GPIO_RBWR_D    0xA9    // GPIO Read-Back/Write Register C

#define GP31STAT_CTL   0x80
#define GP30STAT_CTL   0x40
#define GP29STAT_CTL   0x20
#define GP28STAT_CTL   0x10
#define GP27STAT_CTL   0x08
#define GP26STAT_CTL   0x04
#define GP25STAT_CTL   0x02
#define GP24STAT_CTL   0x01


#define GP_CS_GPIO_CS_MRA  0xB2 // GP_CS to GPIO_CS Map Register A

#define GPCSB_MUX      0xf0
#define GPCSB_MUX3     0x80
#define GPCSB_MUX2     0x40
#define GPCSB_MUX1     0x20
#define GPCSB_MUX0     0x10
#define GPCSA_MUX      0x0f
#define GPCSA_MUX3     0x08
#define GPCSA_MUX2     0x04
#define GPCSA_MUX1     0x02
#define GPCSA_MUX0     0x01


#define GP_CSA_IO_ADR  0xB4    // GP_CSA I/O Address Decode Register

#define CSA_ADDR7      0x80
#define CSA_ADDR6      0x40
#define CSA_ADDR5      0x20
#define CSA_ADDR4      0x10
#define CSA_ADDR3      0x08
#define CSA_ADDR2      0x04
#define CSA_ADDR1      0x02
#define CSA_ADDR0      0x01


#define GP_CSA_IO_ADMR 0xB5    // GP_CSA I/O Address Decode and Mask Register

#define CSA_SA3_MASK   0x20
#define CSA_SA2_MASK   0x10
#define CSA_SA1_MASK   0x08
#define CSA_SA0_MASK   0x04
#define CSA_ADDR9      0x02
#define CSA_ADDR8      0x01


#define GP_CSB_IO_ADR  0xB6    // GP_CSB I/O Address Decode Register

#define CSB_ADDR7      0x80
#define CSB_ADDR6      0x40
#define CSB_ADDR5      0x20
#define CSB_ADDR4      0x10
#define CSB_ADDR3      0x08
#define CSB_ADDR2      0x04
#define CSB_ADDR1      0x02
#define CSB_ADDR0      0x01


#define GP_CSB_IO_ADMR 0xB7    // GP_CSB I/O Address Decode and Mask Register

#define CSB_SA3_MASK   0x20
#define CSB_SA2_MASK   0x10
#define CSB_SA1_MASK   0x08
#define CSB_SA0_MASK   0x04
#define CSB_ADDR9      0x02
#define CSB_ADDR8      0x01


#define GP_CSAB_IO_CQR 0xB8    // GP_CSA/B I/O Command Qualification Register

#define GP_CSB_X8_X16  0x40
#define CSB_GATED_IOX1 0x20
#define CSB_GATED_IOX0 0x10
#define GP_CSA_X8_X16  0x04
#define CSA_GATED_IOX1 0x02
#define CSA_GATED_IOX0 0x01


#define ICR_A          0xD4    // SC410 Interrupt Configuration Register A

#define PIRQ1S         0xf0
#define PIRQ1S3        0x80
#define PIRQ1S2        0x40
#define PIRQ1S1        0x20
#define PIRQ1S0        0x10
#define PIRQ0S         0x0f
#define PIRQ0S3        0x08
#define PIRQ0S2        0x04
#define PIRQ0S1        0x02
#define PIRQ0S0        0x01


#define ICR_B          0xD5    // SC410 Interrupt Configuration Register B

#define PIRQ3S         0xf0
#define PIRQ3S3        0x80
#define PIRQ3S2        0x40
#define PIRQ3S1        0x20
#define PIRQ3S0        0x10
#define PIRQ2S         0x0f
#define PIRQ2S3        0x08
#define PIRQ2S2        0x04
#define PIRQ2S1        0x02
#define PIRQ2S0        0x01


#define ICR_C          0xD6    // SC410 Interrupt Configuration Register C

#define PIRQ5S         0xf0
#define PIRQ5S3        0x80
#define PIRQ5S2        0x40
#define PIRQ5S1        0x20
#define PIRQ5S0        0x10
#define PIRQ4S3        0x08
#define PIRQ4S2        0x04
#define PIRQ4S1        0x02
#define PIRQ4S0        0x01
#define PIRQ4S         0x0f


#define ICR_D          0xD7    // SC410 Interrupt Configuration Register D

#define PIRQ7S         0xf0
#define PIRQ7S3        0x80
#define PIRQ7S2        0x40
#define PIRQ7S1        0x20
#define PIRQ7S0        0x10
#define PIRQ6S         0x0f
#define PIRQ6S3        0x08
#define PIRQ6S2        0x04
#define PIRQ6S1        0x02
#define PIRQ6S0        0x01

#define ICR_E          0xD8    // SC410 Interrupt Configuration Register E


#define SMPSOR         0xE5    // Suspend Mode Pin State Override Register

#define SKTB_OVRD      0x80
#define SKTA_OVRD      0x40
#define SDBUF_OVRD     0x20
#define ISA_OVRD       0x04
#define ROM_OVRD       0x02
#define TERM_LATCH     0x01

#endif /* _MCU_H_ */
