#ifndef __GFX_H__
#define __GFX_H__


IDirectFB * setup_gfx    (void);
void        close_gfx    (void);
void        output_frame (const unsigned char *frame);


#endif /* __GFX_H__ */


