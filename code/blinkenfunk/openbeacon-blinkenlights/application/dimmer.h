#ifndef DIMMER_H
#define DIMMER_H

void vInitDimmer (void);
void vUpdateDimmer (int Percent);
int dimmer_line_hz_enabled(void);
void vSetDimmerGamma (int entry, int val);

#endif /* DIMMER_H */
