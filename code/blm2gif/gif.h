
enum
{
  DISPOSE_UNSPECIFIED,
  DISPOSE_COMBINE,
  DISPOSE_REPLACE
};

void GIFEncodeHeader            (FILE    *fp,
                                 int      gif89,
                                 int      Width,
                                 int      Height,
                                 int      Background,
                                 int      BitsPerPixel,
                                 char    *cmap);
void GIFEncodeGraphicControlExt (FILE    *fp,
                                 int      Disposal,
                                 int      Delay89,
                                 int      NumFramesInImage,
                                 int      Transparent);
void GIFEncodeImageData         (FILE    *fp,
                                 int      Width,
                                 int      Height,
                                 int      BitsPerPixel,
                                 int      offset_x,
                                 int      offset_y,
                                 char    *data);
void GIFEncodeClose             (FILE    *fp);
void GIFEncodeCommentExt        (FILE    *fp,
                                 char    *comment);
void GIFEncodeLoopExt           (FILE    *fp,
                                 int      num_loops);
