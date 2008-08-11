#ifndef __GFX_H__
#define __GFX_H__


void  setup_gfx      (int *argc, char **argv[]);
void  close_gfx      (void);
int   parse_keys     (void);
void  gfx_show_frame (const unsigned char *data);


#endif /* __GFX_H__ */


