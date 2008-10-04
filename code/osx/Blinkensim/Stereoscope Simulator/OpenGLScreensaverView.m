/*
 *
 *  OpenGLScreensaverView.m
 *  OpenGL Cocoa ScreenSaver
 *
 *  Created by tap on November 13 2003.
 *  Copyright (c) 2003 Apple Computer Inc. All rights reserved.
 *
 *  Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
                    ("Apple") in consideration of your agreement to the following terms, and your
                    use, installation, modification or redistribution of this Apple software
                    constitutes acceptance of these terms.  If you do not agree with these terms,
                    please do not use, install, modify or redistribute this Apple software.

                    In consideration of your agreement to abide by the following terms, and subject
                    to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
                    copyrights in this original Apple software (the "Apple Software"), to use,
                    reproduce, modify and redistribute the Apple Software, with or without
                    modifications, in source and/or binary forms; provided that if you redistribute
                    the Apple Software in its entirety and without modifications, you must retain
                    this notice and the following text and disclaimers in all such redistributions of
                    the Apple Software.  Neither the name, trademarks, service marks or logos of
                    Apple Computer, Inc. may be used to endorse or promote products derived from the
                    Apple Software without specific prior written permission from Apple.  Except as
                    expressly stated in this notice, no other rights or licenses, express or implied,
                    are granted by Apple herein, including but not limited to any patent rights that
                    may be infringed by your derivative works or by other works in which the Apple
                    Software may be incorporated.

                    The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
                    WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
                    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
                    PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
                    COMBINATION WITH YOUR PRODUCTS.

                    IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
                    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
                    GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
                    ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
                    OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
                    (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
                    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */



#import "OpenGLScreensaverView.h"
#import "StereoscopeOgreView.h"


@implementation OpenGLScreensaverView

- (id)initWithFrame:(NSRect)frame isPreview:(BOOL)isPreview
{
    self = [super initWithFrame:frame isPreview:isPreview];
    if (self)
    {
        // So we modify the setup routines just a little bit to get our
        // new OpenGL screensaver working.
        
        // Create the new frame
        NSRect newFrame = frame;
        // Slam the origin values
        newFrame.origin.x = 0.0;
        newFrame.origin.y = 0.0;
        // Now alloc and init the view, right from within the screen saver's initWithFrame:
        glView = [[StereoscopeOgreView alloc] initWithFrame:newFrame]; 

        // If the view is valid, we continue
        if(glView)
        {
            // Make sure we autoresize
            [self setAutoresizesSubviews:YES];
            // So if our view is valid...
            if(glView)
                // We make it a subview of the screensaver view
                [self addSubview:glView];
            // Do some setup on our context and view
            //[glView prepareOpenGL];
            // Then we set our animation loop timer
            //[self setAnimationTimeInterval:1/60.0];
            // Since our BasicOpenGLView class does it's setup in awakeFromNib, we call that here.
            // Note that this could be any method you want to use as the setup routine for your view.
            [glView awakeFromNib];
        }
        else // Log an error if we fail here
            NSLog(@"Error: OpenGL Screen Saver failed to initialize NSOpenGLView!");
    }
    // Finally return our newly-initialized self
    return self;
}

- (void)startAnimation
{
    [super startAnimation];
}

- (void)stopAnimation
{
    [super stopAnimation];
}

- (void)drawRect:(NSRect)rect
{
    // And here, we're just calling the drawRect: method in our OpenGL view to render the OpenGL content
    [glView drawRect:rect];
}

- (void)animateOneFrame
{
    // Really simple here - just letting everyone know we need an update
    [glView setNeedsDisplay:YES];
    return;
}

- (BOOL)hasConfigureSheet
{
    return NO;
}

- (NSWindow*)configureSheet
{
    return nil;
}

// Event handling overrides
/*
- (void)keyDown:(NSEvent *)theEvent             
{
    NSLog(@"Characters : %@", [theEvent charactersIgnoringModifiers]);
    if([[theEvent charactersIgnoringModifiers] characterAtIndex:0] == 0x1b)
    {
        [super keyDown:theEvent];
    }
    else
        [glView keyDown:theEvent];
}
- (void)mouseDown:(NSEvent *)theEvent           { [glView mouseDown:theEvent]; }
- (void)rightMouseDown:(NSEvent *)theEvent      { [glView rightMouseDown:theEvent]; }
- (void)otherMouseDown:(NSEvent *)theEvent      { [glView otherMouseDown:theEvent]; }
- (void)mouseUp:(NSEvent *)theEvent             { [glView mouseUp:theEvent]; }
- (void)rightMouseUp:(NSEvent *)theEvent        { [glView rightMouseUp:theEvent]; }
- (void)otherMouseUp:(NSEvent *)theEvent        { [glView otherMouseUp:theEvent]; }
- (void)mouseDragged:(NSEvent *)theEvent        { [glView mouseDragged:theEvent]; }
- (void)scrollWheel:(NSEvent *)theEvent         { [glView scrollWheel:theEvent]; }
- (void)rightMouseDragged:(NSEvent *)theEvent   { [glView rightMouseDragged:theEvent]; }
- (void)otherMouseDragged:(NSEvent *)theEvent   { [glView otherMouseDragged:theEvent]; }
*/
@end
