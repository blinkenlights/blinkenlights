//
//  EAGLView.m
//  Blinkenhall
//
//  Created by Dominik Wagner on 17.05.08.
//  Copyright TheCodingMonkeys 2008. All rights reserved.
//



#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>

#import "EAGLView.h"
#import "BlinkenListener.h"

#define USE_DEPTH_BUFFER 1

@interface UITouch (domadditions)
@property (readonly) CGPoint locationInWindow;
@property (readonly) CGPoint previousLocationInWindow;
@end

@implementation UITouch (domadditions)
- (CGPoint)locationInWindow
{
    return _locationInWindow;
}

- (CGPoint)previousLocationInWindow
{
    return _previousLocationInWindow;
}
@end

// A class extension to declare private methods
@interface EAGLView ()

@property (nonatomic, retain) EAGLContext *context;
@property (nonatomic, assign) NSTimer *animationTimer;

- (BOOL) createFramebuffer;
- (void) destroyFramebuffer;

@end


@implementation EAGLView

@synthesize context;
@synthesize animationTimer;
@synthesize animationInterval;
@synthesize lastFrameData = _lastFrameData;

// You must implement this
+ (Class) layerClass {
	return [CAEAGLLayer class];
}

#define kAccelerometerFrequency		30.0 //Hz
#define kFilteringFactor			0.1

- (void)resetTimeCompensation
{
	NSLog(@"%s",__FUNCTION__);
	_maxTimeDifference = -999999999999999.0;
	_timeSamplesTaken = 0;
}

//The GL view is stored in the nib file. When it's unarchived it's sent -initWithCoder:
- (id)initWithCoder:(NSCoder*)coder {

	if ((self = [super initWithCoder:coder])) {
		_frameQueue = [NSMutableArray new];
		// Get the layer
		CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
		
		eaglLayer.opaque = YES;
		eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
										[NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
		
		context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
		
		if (!context || ![EAGLContext setCurrentContext:context] || ![self createFramebuffer]) {
			[self release];
			return nil;
		}
        
        _windowTexture = [[Texture2D alloc] initWithImagePath:@"Windows256.png"];
        glBindTexture(GL_TEXTURE_2D,_windowTexture.name);
        _pinchChange = 0.6;
        //Configure and start accelerometer
        [[UIAccelerometer sharedAccelerometer] setUpdateInterval:(1.0 / kAccelerometerFrequency)];
        [[UIAccelerometer sharedAccelerometer] setDelegate:self];

		
		// animationInterval = 1.0 / 60.0;
		// [self setNeedsDisplay];
		
		int maxcount = 23*54;
		int i;
		for (i=0;i<maxcount;i++)
		{
			*(((char *)displayState)+i)=-1;
		}
		
		[self resetTimeCompensation];
	}
	return self;
}

- (void) accelerometer:(UIAccelerometer*)accelerometer didAccelerate:(UIAcceleration*)acceleration {
	//Use a basic low-pass filter to only keep the gravity in the accelerometer values
	_accelerometer[0] = acceleration.x * kFilteringFactor + _accelerometer[0] * (1.0 - kFilteringFactor);
	_accelerometer[1] = acceleration.y * kFilteringFactor + _accelerometer[1] * (1.0 - kFilteringFactor);
	_accelerometer[2] = acceleration.z * kFilteringFactor + _accelerometer[2] * (1.0 - kFilteringFactor);
	
	//Render a frame
//	[self setNeedsDisplay];
}


- (void)drawView {
//	NSLog(@"%s",__FUNCTION__);
    const int width = 54;
    
    const int height = 25;
    const GLfloat gapSize = 0.0f;
    const GLfloat windowSize = 1.0f;
    
	const int mask[][54] = 
    {
        {-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, -2,-2,-2,-2,-2,-2,-2,-2,-2,-2, -2,-2, -2,-2, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, -2,-2,-2,-2,-2,-2,-2,-2,-2,-2, -2,-2, -2,-2, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, -2,-2,-2,-2,-2,-2,-2,-2,-2,-2, -2,-2, -2,-2,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, -2,-2,-2,-2,-2,-2,-2,-2,-2,-2, -2,-2, -2,-2,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, -2,-2,-2,-2,-2,-2,-2,-2,-2,-2, -2,-2, -2,-2,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, -2,-2,-2,-2,-2,-2,-2,-2,-2,-2, -2,-2, -2,-2,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1, -1,-1, -2,-2,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1, -1,-1, -2,-2,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, -2,-2,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, -2,-2,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, -2,-2,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, -2,-2,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, -2,-2,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, -2,-2,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, -2,-2, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, -2,-2, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1, -1,-1, -2,-2,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1, -1,-1, -2,-2,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, -2,-2,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, -2,-2,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, -2,-2,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, -2,-2,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, -2,-2,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, -2,-2,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, -2,-2,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    };
	const GLfloat squareVertices[] = {
		-0.5f, -0.5f,
		0.5f,  -0.5f,
		-0.5f,  0.5f,
		0.5f,   0.5f,
	};
	static GLfloat squareTextureCoords[][8] = 
	{
		{
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f,-1.0f,
			1.0f,-1.0f,
		} ,
		{
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f,-1.0f,
			1.0f,-1.0f,
		} ,
		{
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f,-1.0f,
			1.0f,-1.0f,
		} ,
		{
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f,-1.0f,
			1.0f,-1.0f,
		} ,
		{
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f,-1.0f,
			1.0f,-1.0f,
		} ,
		{
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f,-1.0f,
			1.0f,-1.0f,
		} ,
		{
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f,-1.0f,
			1.0f,-1.0f,
		} ,
		{
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f,-1.0f,
			1.0f,-1.0f,
		} ,
		{
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f,-1.0f,
			1.0f,-1.0f,
		} ,
		{
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f,-1.0f,
			1.0f,-1.0f,
		} ,
		{
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f,-1.0f,
			1.0f,-1.0f,
		} ,
		{
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f,-1.0f,
			1.0f,-1.0f,
		} ,
		{
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f,-1.0f,
			1.0f,-1.0f,
		} ,
		{
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f,-1.0f,
			1.0f,-1.0f,
		} ,
		{
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f,-1.0f,
			1.0f,-1.0f,
		} ,
		{
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f,-1.0f,
			1.0f,-1.0f,
		} ,
	};
	
	const GLubyte white[] = 
	{
          255,     255,   255,   255,
          255,     255,   255,   255,
          255,     255,   255,   255,
          255,     255,   255,   255,
	};
	
	static GLubyte squareColors[][16] = 
    {
        {
            0,     0,   0,   0,
            0,     0,   0,   0,
            0,     0,   0,   0,
            0,     0,   0,   0,
        },
        {
          22,     22,   22,   255,
          22,     22,   22,   255,
          22,     22,   22,   255,
          22,     22,   22,   255,
        },
        {
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
        },
        {
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
        },
        {
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
        },
        {
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
        },
        {
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
        },
        {
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
        },
        {
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
        },
        {
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
        },
        {
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
        },
        {
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
        },
        {
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
        },
        {
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
        },
        {
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
        },
        {
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
        },
        {
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
        },
        {
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
        },
        {
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
          255,     255,   255,   20,
        },
    };
	
	static BOOL colorsAreInitialized = NO;
	
	if (!colorsAreInitialized)
	{
		int i = 0;
		int maxColor = 16;
		for (i=0; i<maxColor;i++)
		{
			unsigned char baseValueR = (0xff - 0x03 - 35.) / (maxColor-1.) * i + 0x00;
			unsigned char baseValueG = (0xe4 - 0x0a - 35.) / (maxColor-1.) * i + 0x0a;
			unsigned char baseValueB = (0x6b - 0x17 - 35.) / (maxColor-1.) * i + 0x17;
			int b = 0;
			for (b=0; b<4;b++)
			{
				
				squareColors[i+3][b  ]  = baseValueR + 35;
				squareColors[i+3][b+4]  = baseValueR + 30;
				squareColors[i+3][b+8]  = baseValueR + 25;
				squareColors[i+3][b+12] = baseValueR +  0;

				if (b==1)
				{
					squareColors[i+3][b  ]  = baseValueG + 35;
					squareColors[i+3][b+4]  = baseValueG + 30;
					squareColors[i+3][b+8]  = baseValueG + 25;
					squareColors[i+3][b+12] = baseValueG +  0;
				}

				if (b==2)
				{
					squareColors[i+3][b  ]  = baseValueB + 35;
					squareColors[i+3][b+4]  = baseValueB + 30;
					squareColors[i+3][b+8]  = baseValueB + 25;
					squareColors[i+3][b+12] = baseValueB +  0;
				}
				
				if (b==3)
				{
					squareColors[i+3][b  ]  = 255;
					squareColors[i+3][b+4]  = 255;
					squareColors[i+3][b+8]  = 255;
					squareColors[i+3][b+12] = 255;
				}
			};
			
			int row = i / 4;
			int column = i % 4;
			CGPoint topLeft =     CGPointMake(0.0 + column * 0.25,0.0 + row * 0.25);
			CGPoint bottomRight = CGPointMake(0.0 + (column+1)*0.25, 0.0 + (row+1)*0.25);

			squareTextureCoords[i][0] = topLeft.x;
			squareTextureCoords[i][1] = topLeft.y;
			squareTextureCoords[i][2] = bottomRight.x;
			squareTextureCoords[i][3] = topLeft.y;
			squareTextureCoords[i][4] = topLeft.x;
			squareTextureCoords[i][5] = bottomRight.y;
			squareTextureCoords[i][6] = bottomRight.x;
			squareTextureCoords[i][7] = bottomRight.y;

//		{
//			0.0f, 0.0f,
//			1.0f, 0.0f,
//			0.0f,-1.0f,
//			1.0f,-1.0f,
//		} ,

		}
		colorsAreInitialized = YES;
	}

	[EAGLContext setCurrentContext:context];
	
    GLfloat maxX = (width*windowSize + (width-1) * gapSize) / 2. + windowSize;
    GLfloat maxY = (height*windowSize + (height-1) * gapSize) / 2. + windowSize;
    
    CGRect bounds = [self bounds];
    CGFloat movementFactorX = (maxX * 2) / bounds.size.width;
    CGFloat movementFactorY = (maxY * -2) / bounds.size.height;
    CGFloat movementFactorZ = 1.;
    
    CGFloat displacementZ = _pinchChange * movementFactorZ;
    
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
	glViewport(0, 0, backingWidth, backingHeight);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

    glFrustumf(-maxX,maxX,-maxY, maxY, 0.1, 100.);

//    glOrthof(-maxX, maxX, -maxY, maxY, -maxX, maxX);
    

	glMatrixMode(GL_MODELVIEW);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
    static int startvalue = 0;
    startvalue = (startvalue + 1) % 16;
    

    glLoadIdentity();

	glTranslatef(0, 0, -3.9);
	glTranslatef(0.,0.,displacementZ);
	glScalef(20., 20., 1.);
    
    
    unsigned char *bytes = (unsigned char *)[_lastFrameData bytes];
    int startRow = height - _lastFrameSize.height;
    int rotatingValue = startvalue;
    int x=0;
    for (x=0;x<width;x++) {

        int y=0;
        for (y=0;y<height;y++) {
        	
        	glPushMatrix();
            rotatingValue = (rotatingValue + 1) % 16;

            int value = mask[y][x] + 2;
            
            
			int pictureValue = -1;
			if (y>=2) pictureValue = displayState[y-2][x];
//			if (bytes && _lastFrameSize.width>x && _lastFrameSize.height+startRow>y && y >= startRow) {
//	            pictureValue = ((float)(*(bytes++)) / _maxFrameValue) * 7;
//			}
			
            if (value > 2) {
            	if (pictureValue != -1) {
            		value += pictureValue;
            	} else {
					value += 0;
            	}
            }
            glVertexPointer(2, GL_FLOAT, 0, squareVertices);
            glEnableClientState(GL_VERTEX_ARRAY);
            if (value > 2) {
	            glEnable(GL_TEXTURE_2D);
				glTexCoordPointer(2, GL_FLOAT, 0, squareTextureCoords[value-3]);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glColorPointer(4, GL_UNSIGNED_BYTE, 0, white);
				glEnableClientState(GL_COLOR_ARRAY);
			} else {
				glDisable(GL_TEXTURE_2D);
				glColorPointer(4, GL_UNSIGNED_BYTE, 0, squareColors[value]);
				glEnableClientState(GL_COLOR_ARRAY);
			}
            

		//    glRotatef(_accelerometer[2]/10.,1.,0.,0.);
		//    glTranslatef(_displacement.x * movementFactorX,_displacement.y * movementFactorY,0);

			int xOffset = 0;
			if (_displayMode == 0) {
				glRotatef((x-width/2.+_displacement.x*movementFactorX) * 2. / (float)width,0.,-1.0,0.0);
			} else if (_displayMode == 1) {
				xOffset = +7 + 9;
			}
			 else if (_displayMode == 2) {
				xOffset = -7 - 6;
			}
            
			glScalef(1., _displayMode == 0 ? 0.75 : 0.6, 1.);
	
			glTranslatef(0, 0, -0.9 * _displacement.y / 480.);

			
			int gapWidth = 7;
			glTranslatef(
				(x - width/2. +(x<22?-gapWidth:gapWidth) + xOffset) * (windowSize + gapSize), 
				(height/2. - y) * (windowSize + gapSize),
				0.0);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

//            glTranslatef(0.0,-1. * (windowSize + gapSize),0.0);
       		glPopMatrix();
        }
    }



	// draw accelerometer lines
	
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0., (-height/2.) * (windowSize + gapSize) - gapSize,-0.1);

    GLfloat accelerometerVertices[] = {
        0.,0.5,
        0.,0.5,
        0.,0,
        0.,0,
        0.,-0.5,
        0.,-0.5,
    };

    GLfloat factor = maxX - windowSize*2.;

    accelerometerVertices[2]  =  _accelerometer[0] * factor;
    accelerometerVertices[6]  =  _accelerometer[1] * factor;
    accelerometerVertices[10] =  _accelerometer[2] * factor;
    
    const GLubyte lineColors[] = {
           55,     0,   0,   255,
          255,     0,   0,   255,
          0,      55,   0,   255,
          0,     255,   0,   255,
          0,     0,    55,   255,
          0,     0,   255,   255,
    };
    
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, lineColors);
    glEnableClientState(GL_COLOR_ARRAY);
    
    glVertexPointer(2, GL_FLOAT, 0, accelerometerVertices);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINES, 0, 6);
	
    glPopMatrix();
    
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
	[context presentRenderbuffer:GL_RENDERBUFFER_OES];
}


- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    for (UITouch *touch in touches) {
        if (touch.tapCount > 1)
        {
            _displacement = CGPointZero;
            if (touch.tapCount % 2 == 0) {
				_displayMode = (_displayMode + 1) % 3;
            }

			if (_displayMode == 0) {
				_pinchChange = 0.6;
			} else {
				_pinchChange = 2.3;
			}
        }
    }
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event 
{
    UITouch *touch = [touches anyObject];
    
    if ([[event allTouches] count] == 1)
    {
        CGPoint location = [touch locationInView:self];
        CGPoint previousLocation = [touch previousLocationInView:self];
        _displacement.x += location.x - previousLocation.x;
        _displacement.y += location.y - previousLocation.y;
//		[self setNeedsDisplay];
		[self drawView];
    }
    else if ([[event allTouches] count] == 2)
    {
        BOOL noStarters = YES;
        NSInteger currentTouchIndex = 0;
        CGPoint touchLocations[2];
        CGPoint previousTouchLocations[2];
        for (UITouch *touch in [event allTouches])
        {
            if (touch.phase == UITouchPhaseBegan) {
                 noStarters = NO;
            }
            if (touch.phase == UITouchPhaseMoved) {
                // touch moved handler
            }
            if (touch.phase == UITouchPhaseEnded) {
            }

            // would like to use self, but seems to have a bug if the touch does not have a view
            touchLocations[currentTouchIndex] = touch.locationInWindow;
            // do something with location
            previousTouchLocations[currentTouchIndex] = touch.previousLocationInWindow;
            
            currentTouchIndex++;
        }

        // do some pinching check
        CGPoint differenceBefore = CGPointMake(previousTouchLocations[0].x-previousTouchLocations[1].x,previousTouchLocations[0].y-previousTouchLocations[1].y);
        CGFloat fingerDistanceBefore = sqrt(differenceBefore.x*differenceBefore.x + differenceBefore.y*differenceBefore.y);
        CGPoint differenceAfter = CGPointMake(touchLocations[0].x-touchLocations[1].x,touchLocations[0].y-touchLocations[1].y);
        CGFloat fingerDistanceAfter  = sqrt(differenceAfter.x*differenceAfter.x + differenceAfter.y*differenceAfter.y);
        CGFloat distanceChange = fingerDistanceAfter - fingerDistanceBefore;
        if (ABS(distanceChange) > 1.)
        {
            _pinchChange += distanceChange / 300.;
            NSLog(@"%s pc:%f dc:%f",__FUNCTION__,_pinchChange, distanceChange);
//			[self setNeedsDisplay];
			[self drawView];
        }
    }
    
    
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{

}


- (void)layoutSubviews {
	[EAGLContext setCurrentContext:context];
	[self destroyFramebuffer];
	[self createFramebuffer];
	[self drawView];
}


- (BOOL)createFramebuffer {
	
	glGenFramebuffersOES(1, &viewFramebuffer);
	glGenRenderbuffersOES(1, &viewRenderbuffer);
	
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
	[context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(id<EAGLDrawable>)self.layer];
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, viewRenderbuffer);
	
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);
	
	if (USE_DEPTH_BUFFER) {
		glGenRenderbuffersOES(1, &depthRenderbuffer);
		glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthRenderbuffer);
		glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, backingWidth, backingHeight);
		glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthRenderbuffer);
	}

	if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
		NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
		return NO;
	}
	
	return YES;
}


- (void)destroyFramebuffer {
	
	glDeleteFramebuffersOES(1, &viewFramebuffer);
	viewFramebuffer = 0;
	glDeleteRenderbuffersOES(1, &viewRenderbuffer);
	viewRenderbuffer = 0;
	
	if(depthRenderbuffer) {
		glDeleteRenderbuffersOES(1, &depthRenderbuffer);
		depthRenderbuffer = 0;
	}
}


- (void)startAnimation {
	self.animationTimer = [NSTimer scheduledTimerWithTimeInterval:animationInterval target:self selector:@selector(drawView) userInfo:nil repeats:YES];
}


- (void)stopAnimation {
	self.animationTimer = nil;
}


- (void)setAnimationTimer:(NSTimer *)newTimer {
	[animationTimer invalidate];
	animationTimer = newTimer;
}


- (void)setAnimationInterval:(NSTimeInterval)interval {
	
	animationInterval = interval;
	if (animationTimer) {
		[self stopAnimation];
		[self startAnimation];
	}
}

- (void)consumeFrame
{
	NSArray *frames = [_frameQueue lastObject];
	for (BlinkenFrame *frame in frames)
	{
		CGSize frameSize = frame.frameSize;
		NSData *frameData = frame.frameData;
		BOOL isNibbles = frame.bitsPerPixel != 8;
		unsigned char *bytes = (unsigned char *)frameData.bytes;
		unsigned char *bytesEnd = bytes + frameData.length;
		NSUInteger bytesPerRow = (NSUInteger)frameSize.width;
		unsigned char maxValue = frame.maxValue;
		if (isNibbles) bytesPerRow = (bytesPerRow + 1) / 2;

		unsigned char screenID = frame.screenID;

		if (screenID == 0)
		{
			if ((int)frameSize.width == 96 && (int)frameSize.height == 32)
			{
				int rowIndex = 0;
				for (;bytes < bytesEnd;bytes+=bytesPerRow,rowIndex++)
				{
					if (rowIndex >= 5)
					{
						if (isNibbles)
						{
							int offset = 8;
							for (offset = 8; offset < 8+11; offset++)
							{
								displayState[rowIndex - 5][(offset-8)*2]   = (((int)*(bytes + offset)) >>  4) * 15 / maxValue;
								displayState[rowIndex - 5][(offset-8)*2+1] = (((int)*(bytes + offset)) & 0xf) * 15 / maxValue;
							}
							for (offset = 25; offset < 25+15; offset++)
							{
								displayState[rowIndex - 5][24+(offset-25)*2]   = (((int)*(bytes + offset)) >>  4) * 15 / maxValue;
								displayState[rowIndex - 5][24+(offset-25)*2+1] = (((int)*(bytes + offset)) & 0xf) * 15 / maxValue;
							}
						}
						else
						{
							int offset = 16;
							for (offset = 16; offset < 16+22; offset++)
							{
								displayState[rowIndex - 5][offset - 16] = ((int)*(bytes + offset)) * 15 / maxValue;
							}
							for (offset = 50; offset < 50+30; offset++)
							{
								displayState[rowIndex - 5][offset - 50 + 24] = ((int)*(bytes + offset)) * 15 / maxValue;
							}
						}
					}
					if (rowIndex >= 31 - 4) break;
				}
			}
		} 
		else if (screenID == 5)
		{
			// this is the left tower probably
			if ((int)frameSize.width == 22 && (int)frameSize.height == 17)
			{
				int rowIndex = 0;
				for (;bytes < bytesEnd;bytes+=bytesPerRow,rowIndex++)
				{
					if (isNibbles)
					{
						int x = 0;
						for (x = 0; x < 11; x++)
						{
							displayState[rowIndex + 6][x*2]   = (((int)*(bytes + x)) >>  4) * 15 / maxValue;
							displayState[rowIndex + 6][x*2+1] = (((int)*(bytes + x)) & 0xf) * 15 / maxValue;
						}
					}
					else
					{
						int x = 0;
						for (x = 0; x < 22; x++)
						{
							displayState[rowIndex + 6][x] = ((int)*(bytes + x)) * 15 / maxValue;
						}
					}
				}
			}
		}
		else if (screenID == 6)
		{
			// this is the right tower probably
			if ((int)frameSize.width == 30 && (int)frameSize.height == 23)
			{
				int rowIndex = 0;
				for (;bytes < bytesEnd;bytes+=bytesPerRow,rowIndex++)
				{
					if (isNibbles)
					{
						int x = 0;
						for (x = 0; x < 15; x++)
						{
							displayState[rowIndex][24+x*2]   = (((int)*(bytes + x)) >>  4) * 15 / maxValue;
							displayState[rowIndex][24+x*2+1] = (((int)*(bytes + x)) & 0xf) * 15 / maxValue;
						}
					}
					else
					{
						int x = 0;
						for (x = 0; x < 30; x++)
						{
							displayState[rowIndex][24+x] = ((int)*(bytes + x)) * 15 / maxValue;
						}
					}
				}
			}
		}
	}
	[_frameQueue removeLastObject];
}

- (void)blinkenListener:(BlinkenListener *)inListener receivedFrames:(NSArray *)inFrames atTimestamp:(uint64_t)inTimestamp
{	
	[_frameQueue insertObject:inFrames atIndex:0];

	if (inTimestamp != 0) 
	{
		NSTimeInterval now = [[NSDate date] timeIntervalSince1970] ;
		NSTimeInterval timeDifference = now - ((NSTimeInterval)inTimestamp / 1000.0);
		NSTimeInterval compensationTime = (_maxTimeDifference - timeDifference);
		if (_timeSamplesTaken > 5 && ABS(timeDifference - _maxTimeDifference) > 3.0) 
		{
			[self resetTimeCompensation];
		}
		_maxTimeDifference = MAX(timeDifference,_maxTimeDifference);
		if (_timeSamplesTaken <= 5 && compensationTime > 0)
		{
			_timeSamplesTaken++;
			[self consumeFrame];
		}
		else
		{
			// compensate
			[self performSelector:@selector(consumeFrame) withObject:nil afterDelay:compensationTime];
			_maxTimeDifference -= 0.01; // shrink the time difference again to catch up if we only had one hickup
		}
		// NSLog(@"%s ts:0x%016qx %@ now: %@ 0x%016qx",__FUNCTION__,inTimestamp,[NSDate dateWithTimeIntervalSince1970:inTimestamp / (double)1000.0],now, (uint64_t)([now timeIntervalSince1970] * 1000));
		//NSLog(@"time difference: %0.5f s - compensation %0.5f s",timeDifference,compensationTime);
	} else {
		[self consumeFrame];
	}
}

//- (void)receivedFrameData:(NSData *)inFrameData ofSize:(CGSize)inSize channels:(int)inChannels maxValue:(unsigned char)inMaxValue
//{
//	self.lastFrameData = inFrameData;
//	_lastFrameSize     = inSize;
//	_numberOfChannels  = inChannels;
//	_maxFrameValue     = inMaxValue;
////	[self setNeedsDisplay];
//	unsigned char *bytes = (unsigned char *)[inFrameData bytes];
//	if (inSize.width == 96 && inSize.height == 32) {
//		int sourceY = 5;
//		int y=0;
//		for (y=0;y<23;y++,sourceY++)
//		{
//			int x=0;
//			int sourceX = 16;
//			for (x=0;x<22;x++,sourceX++)
//			{
//				displayState[y][x]= (*(bytes+sourceY*96+sourceX)) * 15 / inMaxValue;
//			}
//			sourceX = 50;
//			for (x=24;x<54;x++,sourceX++)
//			{
//				displayState[y][x]=(*(bytes+sourceY*96+sourceX)) * 15 / inMaxValue;
//			}
//		}
//	} else {
//		int yPadding = 0;
//		if (inSize.height < 23) yPadding = 23-inSize.height;
//		int y = 0;
//		for (y=0;y<23;y++) {
//			int x = 0;
//			for (x=0;x<54;x++) {
//				displayState[y][x]=(y >= yPadding && x < inSize.width && y-yPadding<inSize.height) ? (*(bytes+(y-yPadding)*(int)inSize.width+x)) * 15 / inMaxValue : -1;
//			}
//		}
//	}
//	//[self drawView];
//}


- (void)dealloc {
	
	[self stopAnimation];
	
	if ([EAGLContext currentContext] == context) {
		[EAGLContext setCurrentContext:nil];
	}

	[_frameQueue release];
	[context release];	
	[super dealloc];
}

@end
