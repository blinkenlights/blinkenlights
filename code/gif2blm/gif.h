
enum
{
  DISPOSE_UNSPECIFIED,
  DISPOSE_COMBINE,
  DISPOSE_REPLACE
};

typedef enum
{
  IMAGE,
  GRAPHIC_CONTROL_EXTENSION,
  COMMENT_EXTENSION,
  UNKNOWN_EXTENSION,
  TERMINATOR
} GIFRecordType;

int  GIFDecodeHeader            (FILE           *fd,
                                 int            *Width,
                                 int            *Height,
                                 int            *Background,
                                 int            *colors,
                                 unsigned char **cmap);
int  GIFDecodeRecordType        (FILE           *fd,
                                 GIFRecordType  *type);
int  GIFDecodeImage             (FILE           *fd,
                                 int            *Width,
                                 int            *Height,
                                 int            *offx,
                                 int            *offy, 
                                 int            *colors,
                                 unsigned char **cmap,
                                 unsigned char  *data);
int  GIFDecodeGraphicControlExt (FILE           *fd,
                                 int            *Disposal,
                                 int            *Delay,
                                 int            *Transparent);
int  GIFDecodeCommentExt        (FILE           *fd,
                                 char          **comment);
void GIFDecodeUnknownExt        (FILE           *fd);
