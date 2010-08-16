#import "BlinkenImageProvider.h"


@implementation BlinkenImageProvider

@synthesize frameData = _frameData;
@synthesize bitsPerPixel = _bitsPerPixel;

- (void)dealloc
{
	self.frameData = nil;
	[super dealloc];
}

//- (BOOL) shouldColorMatch
//{
//	return NO;
//}
//
- (void)setFrameData:(NSData *)inFrameData size:(CGSize)inSize channels:(int)inChannels maxValue:(unsigned char)inMaxValue bitsPerPixel:(unsigned char)inBitsPerPixel
{
    @synchronized (self) {
        self.frameData = inFrameData;
        _frameSize = NSSizeFromCGSize(inSize);
        _numberOfChannels = inChannels;
        _maxValue = inMaxValue;
        _bitsPerPixel = inBitsPerPixel;
    }
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
    @synchronized (self) {
//    	NSLog(@"%s %@",__FUNCTION__,NSStringFromRect(inBounds));
        BOOL isBGRA = [QCPlugInPixelFormatBGRA8 isEqualToString:inFormat];
    	int rowOffset = _frameSize.height - NSMaxY(inBounds);
        unsigned char *src, *dest;
        src = (unsigned char *)[self.frameData bytes];
		unsigned color_offset = _frameSize.height * _frameSize.width;
        dest = inBaseAddress;
        NSUInteger myRowBytes = (_bitsPerPixel == 4) ? (((int)_frameSize.width) + 1) / 2 : _frameSize.width;
    
        NSUInteger numberOfRows = MIN(_frameSize.height, inBounds.size.height);
    
        unsigned char alpha = 255;
    
        // currentrow = 0 is the first row to be copied, not row with y coordinate 0
        int currentRow = inBounds.origin.y;
        for (currentRow = inBounds.origin.y;currentRow < numberOfRows;currentRow++) {
            unsigned char *sourceRow = 
            	src + 
            	currentRow * myRowBytes + 
            	rowOffset * myRowBytes +
            	(_bitsPerPixel == 4 ? ((int)inBounds.origin.x / 2) : ((int)inBounds.origin.x));
            unsigned char *destRow = dest + (currentRow)* inRowBytes;
            int x = 0;
            for (x=0;x<MIN(_frameSize.width, inBounds.size.width);x++) {
				
                unsigned char red = 0, green = 0, blue = 0;
                if (_bitsPerPixel == 4) {
                	if (x % 2 == 0) {
                		red = green = blue = ((*sourceRow) >> 4);
                	} else {
	                	red = green = blue = ((*sourceRow) & 0xF);
	                	sourceRow++;
	                }
                	red *= 0x11;
					green = blue = red;
                } else {
                	red = (*sourceRow) * 255. / _maxValue;
					if (_numberOfChannels == 3) {
						green = (*(sourceRow + color_offset)) * 255. / _maxValue;
						blue = (*(sourceRow + color_offset + color_offset)) * 255. / _maxValue;
					} else {
						green = blue = red;
					}
                	sourceRow++;
                }
                if (isBGRA) {
                    *destRow = blue; destRow++;  
                    *destRow = green; destRow++; 
                    *destRow = red; destRow++; 
                    *destRow = alpha;     destRow++; 
                } else {
                    *destRow = alpha;     destRow++; 
                    *destRow = red; destRow++;  
                    *destRow = green; destRow++;  
                    *destRow = blue; destRow++;  
                }
            }
        }	
    }
	return YES;
}


- (CGColorSpaceRef) imageColorSpace {
	return [[NSColorSpace deviceRGBColorSpace] CGColorSpace];
}


- (NSArray*) supportedBufferPixelFormats
{
	NSArray *result = [NSArray arrayWithObjects:QCPlugInPixelFormatARGB8,QCPlugInPixelFormatBGRA8, nil];
	return result;
}

@end
