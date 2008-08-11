/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "values.h"


static const int exp_table[256] = {
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 9, 10, 10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 19, 19, 20, 20, 21, 22, 22, 23, 24, 25, 26, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 39, 40, 41, 42, 44, 45, 47, 48, 50, 51, 53, 55, 56, 58, 60, 62, 64, 66, 68, 70, 73, 75, 77, 80, 82, 85, 88, 90, 93, 96, 99, 103, 106, 109, 113, 116, 120, 124, 128, 132, 136, 140, 145, 149, 154, 159, 164, 169, 175, 180, 186, 192, 198, 204, 211, 218, 225, 232, 239, 247, 255 };

	
void apply_voodoo (void) {
	int x, y, i, v;
	unsigned char vals[WIDTH][2];

	for (x = 0; x < WIDTH; x++)
		for (y = 0; y < 2; y++)
			if (audio_vals[x][y] > 0xf && audio_vals[x][y] > midi_vals[x][y])
				vals[x][y] = audio_vals[x][y];
			else  	vals[x][y] = midi_vals[x][y];
	
	if (flags & FLAG_FLIP && mode != MODE_VUMETER)
		for (x=0; x<WIDTH/2; x++) {
			register int t;
			
			t = vals[x][0];
			vals[x][0] = vals[WIDTH-x-1][0];
			vals[WIDTH-x-1][0] = t;
	
			t = vals[x][1];
			vals[x][1] = vals[WIDTH-x-1][1];
			vals[WIDTH-x-1][1] = t;
		}
	
	switch (mode) {
		case MODE_VANALYZER:
			for (x=0; x<WIDTH; x++) {
				int dest;
				if (flags & FLAG_STEREO) {
					if (x%2 == 0) {
						dest = x/2;
						v = (vals[x][1] + vals[x+1][1]) / 2;
					} else {
						dest = (WIDTH-(x/2)-1);
						v = (vals[x][0] + vals[x-1][0]) / 2;
					}
				} else {
					dest = x;
					v = (vals[x][0] + vals[x][1])/2;
				}
		
				if (flags & FLAG_FOUNTAIN) {
					if (v >= 0xAA) {
						matrix[2][dest] = v/0xd + 0xec;
						matrix[1][dest] = v;
						matrix[0][dest] = (v-0xaa) * 2 + 0x55;
					} else if (v >= 0x55) {
						matrix[2][dest] = (v-0x55)/2 + 0xaa; 
						matrix[1][dest] = v;
						matrix[0][dest] = (v-0x55)/2 + 0xf;
					} else {
						matrix[2][dest] = v*2;
						matrix[1][dest] = v;
						matrix[0][dest] = v/0x6;
					}
				} else {
					matrix[0][dest] = matrix[1][dest] = matrix[2][dest] = v;
				}
				continue;

                                v <<= 1;
                                if (v & 0x80) {
                                        matrix[0][dest] = 0;
                                        matrix[1][dest] = v << 1;
                                        matrix[2][dest] = v;
                                        continue;
                                }

                                v <<= 1;
                                matrix[0][dest] = 0;
                                matrix[1][dest] = 0;
                                matrix[2][dest] = v << 1;
                        }
                        break;
		
		case MODE_HANALYZER:
			for (y=0; y<HEIGHT; y++) {
				v = 0;
				for (x = (y*(WIDTH/HEIGHT));
                                     x < (y+1) * (WIDTH/HEIGHT);
                                     x++)
					v += vals[x][0] + vals[x][1];
				v /= 2*WIDTH/HEIGHT;
				
				if (flags & FLAG_FOUNTAIN)
					if (!(flags & FLAG_FLIP)) {
						for (x = 0; x < WIDTH; x++)
							matrix[HEIGHT-y-1][x] = (v >= (x*0xfd)/WIDTH)?v:0;
					} else {
						for (x = 0; x < WIDTH; x++)
							matrix[HEIGHT-y-1][WIDTH-1-x] = (v >= (x*0xfd)/WIDTH)?v:0;
					}
				else
					for (x = 0; x < WIDTH; x++)
						matrix[HEIGHT-y-1][x] = v;
			}
			break;
		
		case MODE_VUMETER:
                        if (flags & FLAG_STEREO) {
                                for (x=0, v=0; x < WIDTH; x++)
                                       v += vals[x][0];
				v = exp_table[v/WIDTH];
                                v >>= 1;
				
                                if (flags & FLAG_FLIP) {
                                        for (x = 0; x < v - 3; x++)
                                                matrix[0][x] = 0xFF;
                                        for (i = 0; x < v + 3; x++, i++)
                                                matrix[0][x] = (0xFF*(6-i))/6;
                                        for (; x < WIDTH/2; x++)
                                                matrix[0][x] = 0;
                                } else {
                                        v = WIDTH/2 - v;
                                        for (x = 0; x < v - 3; x++)
                                                matrix[0][x] = 0;
                                        for (i = 0; x < v + 3; x++, i++)
                                                matrix[0][x] = (0xFF*(i+1))/6;
                                        for (; x < WIDTH/2; x++)
                                                matrix[0][x] = 0xFF;
                                }

                                for (x=0, v=0; x < WIDTH; x++)
                                       v += vals[x][1];
				v = exp_table[v/WIDTH];
                                v >>= 1;

                                if (flags & FLAG_FLIP) {
                                        v = WIDTH/2 - v;
                                        for (x = WIDTH/2; x - WIDTH/2 < v - 3; x++)
                                                matrix[0][x] = 0;
                                        for (i = 0; x - WIDTH/2 < v + 3; x++, i++)
                                                matrix[0][x] = (0xFF*(i+1))/6;
                                        for (; x < WIDTH; x++)
                                                matrix[0][x] = 0xFF;
                                } else {
                                        for (x = WIDTH/2; x - WIDTH/2 < v - 3; x++)
                                                matrix[0][x] = 0xFF;
                                        for (i = 0; x - WIDTH/2 < v + 3; x++, i++)
                                                matrix[0][x] = (0xFF*(6-i))/6;
                                        for (; x < WIDTH; x++)
                                                matrix[0][x] = 0;
                                }

                        } else {  /* !(flags & FLAG_STEREO) */

                                for (x=0, v=0; x < WIDTH; x++)
                                       v += vals[x][0] + vals[x][1];
				v = exp_table[v/WIDTH/2];

                                if (flags & FLAG_FLIP) {
                                        v = WIDTH - v;
                                        for (x = 0; x < v - 6; x++)
                                                matrix[0][x] = 0;
                                        for (i = 0; x < v + 6; x++, i++)
                                                matrix[0][x] = (0xFF*(i+1))/12;
                                        for (; x < WIDTH; x++)
                                                matrix[0][x] = 0xFF;
                                } else {
                                        for (x = 0; x < v - 6; x++)
                                                matrix[0][x] = 0xFF;
                                        for (i = 0; x < v + 6; x++, i++)
                                                matrix[0][x] = (0xFF*(12-i))/12;
                                        for (; x < WIDTH; x++)
                                                matrix[0][x] = 0;
                                }
                        }
                        memcpy (matrix[1], matrix[0], WIDTH);
                        memcpy (matrix[2], matrix[0], WIDTH);
			break;

		default:
			mode = MODE_VANALYZER;
			break;
	}

//	for (x=0; x<WIDTH; x++)
//		printf ("%02d: %d\n", x, vals[x][0]);
}

