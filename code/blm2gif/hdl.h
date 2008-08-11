#ifndef __HDL_H__
#define __HDL_H__

typedef enum
{
  IMAGE_PLAIN,
  IMAGE_HDL_SMALL,
  IMAGE_HDL_MEDIUM,
  IMAGE_HDL_LARGE
} ImageType;

typedef struct 
{
  int   width;
  int   height;
  int   offx;
  int   offy;
  int   dx;
  int   dy;
  int   ncolors;
  int   cbits;
  char *colors; 
  char *on; 
  char *off; 
} HdlInfo;


static char hdl_plain_colors[6] = { 0x0, 0x0, 0x0, 0xFF, 0xFF, 0xFF };
  
static HdlInfo hdl_plain =
{
  0, 0,
  0, 0,
  1, 1,
  2, 1, hdl_plain_colors,
  NULL, NULL
};

#endif /* __HDL_H__ */
