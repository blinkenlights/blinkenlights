#ifndef __MAGIC_VALUES_H__
#define __MAGIC_VALUES_H__

#define PORT                   2323
#define WIDTH                    16
#define HEIGHT                    3
#define MAGIC_HDL_FRAME  0xDEADBEEF
#define MAGIC_HDL2_FRAME 0xFEEDBEEF
#define MAGIC_MCU_FRAME  0x23542666

#define KEYBOARD_TIMEOUT     300000

#ifndef FALSE
#define FALSE 0
#define TRUE (!FALSE)
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#endif /* __MAGIC_VALUES_H__ */
