#ifndef _MIDI_H_
#define _MIDI_H_

#define MIDIDEV "/dev/midi00"
#define KEYSTART_MODE 0
#define KEYSTART_FLAGS (KEYSTART_MODE+12)
#define KEYSTART_DIRECT 48

#define CHROMATIC_KEYS 0

extern int midi_control;
extern int midi_map[0x7f][0x10];

int midi_open (void);
int midi_parse (unsigned char *buf, int size);
void midi_install_default_map (void);

#endif /* _MIDI_H_ */
