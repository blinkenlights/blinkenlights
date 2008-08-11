#ifndef _VALUES_H_
#define _VALUES_H_

#define WIDTH  16
#define HEIGHT 3
#define MAX_MODE 12
#define MAX_FLAGS 12

#define MAX(a,b) ((a>b)?a:b)

extern unsigned char audio_vals[WIDTH][2];
extern unsigned char midi_vals[WIDTH*2][2];
extern unsigned char matrix[HEIGHT][WIDTH];
extern int mode, flags;

enum {
	MODE_VANALYZER  = 0,
	MODE_HANALYZER  = 1,
	MODE_VUMETER    = 2
};

enum {
	FLAG_PAUSE 	= 1 << 0,
        FLAG_CLEAR      = 1 << 1,
	FLAG_FLIP	= 1 << 2,
	FLAG_STEREO	= 1 << 3,
	FLAG_FOUNTAIN	= 1 << 4
};

#endif /* _VALUES_H_ */
