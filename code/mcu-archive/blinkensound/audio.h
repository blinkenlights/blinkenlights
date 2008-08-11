#ifndef _AUDIO_H_
#define _AUDIO_H_

#define AUDIODEV "/dev/dsp"

int audio_open (void);
int audio_parse (short int *buf);

int audio_buf_size;
int audio_channels;

#endif /* _AUDIO_H_ */
