#ifndef _AUDIO_H_
#define _AUDIO_H_

#define AUDIODEV "/dev/audio"

gint b_audio_open (void);
gint b_audio_parse (gshort *buf,
		    guchar *dest,
		    guint   width);

#endif /* _AUDIO_H_ */
