#import "BlinkenImageProvider.h"


@implementation BlinkenImageProvider

@synthesize frameData = _frameData;

- (void)dealloc
{
	self.frameData = nil;
	[super dealloc];
}

- (void)setFrameData:(NSData *)inFrameData size:(CGSize)inSize channels:(int)inChannels maxValue:(unsigned char)inMaxValue
{
	self.frameData = inFrameData;
	_frameSize = NSSizeFromCGSize(inSize);
	_numberOfChannels = inChannels;
	_maxValue = inMaxValue;
}

- (BOOL)canRenderWithCGLContext:(CGLContextObj)cgl_ctx
{
	return NO;
}

- (NSRect)imageBounds
{
//	NSLog(@"%s",__FUNCTION__);
	return NSMakeRect(0.,0.,_frameSize.width,_frameSize.height);
}

- (BOOL)renderToBuffer:(void*)inBaseAddress withBytesPerRow:(NSUInteger)inRowBytes pixelFormat:(NSString*)inFormat forBounds:(NSRect)inBounds
{
//	NSLog(@"%s",__FUNCTION__);
	BOOL isBGRA = [QCPlugInPixelFormatBGRA8 isEqualToString:inFormat];

	unsigned char *src, *dest;
	src = (unsigned char *)[self.frameData bytes];
	dest = inBaseAddress;
	NSUInteger myRowBytes = _frameSize.width;

    NSUInteger numberOfRows = inBounds.size.height;

	unsigned char alpha = 255;

    
    int currentRow = inBounds.origin.y;
    for (currentRow = inBounds.origin.y;currentRow < numberOfRows;currentRow++) {
		unsigned char *sourceRow = src + currentRow * myRowBytes;
		unsigned char *destRow = dest + currentRow * inRowBytes;
        int x = 0;
        for (x=0;x<inBounds.size.width;x++) {
        
            unsigned char grayValue = (*sourceRow) * 255. / _maxValue;
            sourceRow++;
            if (isBGRA) {
                *destRow = grayValue; destRow++;  
                *destRow = grayValue; destRow++; 
                *destRow = grayValue; destRow++; 
                *destRow = alpha;     destRow++; 
            } else {
                *destRow = alpha;     destRow++; 
                *destRow = grayValue; destRow++;  
                *destRow = grayValue; destRow++;  
                *destRow = grayValue; destRow++;  
            }
        }
    }	
	return YES;
}

- (CGColorSpaceRef) imageColorSpace {
	return [[NSColorSpace genericRGBColorSpace] CGColorSpace];
}


- (NSArray*) supportedBufferPixelFormats
{
	NSArray *result = [NSArray arrayWithObjects:QCPlugInPixelFormatARGB8,QCPlugInPixelFormatBGRA8, nil];
	return result;
}

@end
