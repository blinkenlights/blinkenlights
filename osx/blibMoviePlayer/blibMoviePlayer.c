#include <stdio.h>
#include <QuickTime/QuickTime.h>
#include <blib/blib.h>



float gWidth=96.;
float gHeight=32.;

BSender *gSender;
guchar *gData;

#define ESC	27

// ASCII pixel value array
char convert[256];

// DrawCompleteProc - After the frame has been drawn QuickTime calls us to do some work
static pascal OSErr DrawCompleteProc(Movie theMovie, long refCon)
{
	static int skipper = 0;
	int	y, x;
	GWorldPtr	offWorld = (GWorldPtr)refCon;
	Rect		bounds;
	Ptr			baseAddr;
	long		rowBytes;
	
	// get the information we need from the GWorld
	GetPixBounds(GetGWorldPixMap(offWorld), &bounds);
	baseAddr = GetPixBaseAddr(GetGWorldPixMap(offWorld));
	rowBytes = QTGetPixMapHandleRowBytes(GetGWorldPixMap(offWorld));

	// goto home
	printf("%c[0;0H", ESC);
	
	// for each row
	for (y = 0; y < gHeight; ++y) {
		long	*p;
		
		// for each pixel
		p = (long*)(baseAddr + rowBytes * (long)(y * ((bounds.bottom - bounds.top) / gHeight)));
		for (x = 0; x < gWidth; ++x) {
			UInt32			color;
			long			Y;
			long			R;
			long			G;
			long			B;

			color = *(long *)((long)p + 4 * (long)(x*(bounds.right - bounds.left) / gWidth));
			R = (color & 0x00FF0000) >> 16;
			G = (color & 0x0000FF00) >> 8;
			B = (color & 0x000000FF) >> 0;

			// convert to YUV for comparison
			// 5 * R + 9 * G + 2 * B
			Y = (R + (R << 2) + G + (G << 3) + (B + B)) >> 4;
			
			// draw it
			gData[y*(int)gWidth + x] = Y;
			putchar(convert[Y]);
		}
		
		// next line
		putchar('\n');
	}
	
	if (skipper == 0) b_sender_send_frame (gSender, gData);
	skipper = (skipper + 1) % 1;
	return noErr;
}

int main(int argc, char *argv[])
{
	if (argc < 3 || argc > 5)
	{
		printf("usage %s <movie filename> <hostname> [width] [height]\n",argv[0]);
		return 0;
	}

	if (argc == 5)
	{
		gWidth = atoi(argv[3]);
		gHeight = atoi(argv[4]);
	}
	else if (argc == 4)
	{
		gWidth = atoi(argv[3]);
		gHeight = gWidth / 4. * 3.;
	}

	if (gWidth < 1.) 
	{
		gWidth = 18.;
	}
	
	if (gHeight < 1.)
	{
		gHeight = 8;
	}

	MovieController	thePlayer = NULL;
	Movie		theMovie = NULL;
	GWorldPtr	offWorld;
	Rect		bounds;
	short       actualResId = DoTheRightThing;
	int i;
    OSErr		result = 0;
	MovieDrawingCompleteUPP	myDrawCompleteProc = NewMovieDrawingCompleteUPP(DrawCompleteProc);
    
	// Using Data Reference calls now
	OSType      myDataRefType;
    Handle      myDataRef = NULL;
	CFStringRef inPath;
	
	/* build the luminance value to ASCII value conversion table
	*/
	for (i = 0; i < 256; ++i) {
		char *table = " .;+o8#@";
		convert[i] = table[i * strlen(table) / 256];
	}

	GError  *error = NULL;

	// initialize blib
	b_init();
	gSender = b_sender_new();
	g_assert(gSender);
	
	if (!b_sender_add_recipient (gSender,-1,argv[2],MCU_LISTENER_PORT, &error)) {
		g_printerr (error->message);
		return EXIT_FAILURE;
	}
	
	b_sender_configure (gSender, gWidth, gHeight, 1, 255);
	gData = g_new(guchar,((int)gWidth) * ((int)gHeight));

	EnterMovies();

	// home
	printf("%c[0;0H", ESC);
	// erase to end of display
	printf("%c[0J", ESC);

	// Convert movie path to CFString
	inPath = CFStringCreateWithCString(NULL, argv[1], CFStringGetSystemEncoding());
    if (!inPath) { printf("Could not get CFString \n"); goto bail; }
	
	// create the data reference
    result = QTNewDataReferenceFromFullPathCFString(inPath, kQTNativeDefaultPathStyle,
                                                    0, &myDataRef, &myDataRefType);
    if (result) { printf("Could not get DataRef %d\n", result); goto bail; }

    // get the Movie
    result = NewMovieFromDataRef(&theMovie, newMovieActive,
                                 &actualResId, myDataRef, myDataRefType);
    if (result) { printf("Could not get Movie from DataRef %d\n", result); goto bail; }

    // dispose the data reference handle - we no longer need it
    DisposeHandle(myDataRef);

	GetMovieBox(theMovie, &bounds);
    
    // use kNativeEndianPixMap flag so we do the right thing on Intel-based macs
	QTNewGWorld(&offWorld, k32ARGBPixelFormat, &bounds, NULL, NULL, kNativeEndianPixMap);
	LockPixels(GetGWorldPixMap(offWorld));
	SetGWorld(offWorld, NULL);
	
	thePlayer = NewMovieController(theMovie, &bounds, mcTopLeftMovie | mcNotVisible);
	SetMovieGWorld(theMovie, offWorld, NULL);
	SetMovieActive(theMovie, true);
	SetMovieDrawingCompleteProc(theMovie, movieDrawingCallWhenChanged, myDrawCompleteProc, (long)offWorld); 
	MCDoAction(thePlayer, mcActionPrerollAndPlay, (void *)Long2Fix(1));
	
	do {
		MCIdle(thePlayer);
	} while (!IsMovieDone(theMovie));
	
bail:
	g_free(gData);
	g_object_unref(gSender);

    if (thePlayer) DisposeMovieController(thePlayer);
    if (theMovie) DisposeMovie(theMovie);
    if (myDrawCompleteProc) DisposeMovieDrawingCompleteUPP(myDrawCompleteProc);
		
    return result;
}
