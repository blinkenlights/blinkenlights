#ifndef _BSOUND_H_
#define _BSOUND_H_

#define WIDTH 26
#define HEIGHT 20
#define MAX_MODE 1
#define MAX_FLAGS 1
#define CHANNELS 2

#define DEFAULT_PORT 2323
#define AUDIO_RBUF_SIZE 20

#define AUDIO_PEAK_TIMEOUT 5

extern guchar *audio_vals[AUDIO_RBUF_SIZE];
extern guchar  audio_peak[WIDTH];
extern guchar  audio_peak_timeout[WIDTH];
extern guchar  audio_peak_color[WIDTH];
extern guchar  midi_vals[WIDTH];
extern guchar  matrix[HEIGHT][WIDTH];
extern gint    mode, flags;

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

#endif /* _BSOUND_H_ */
