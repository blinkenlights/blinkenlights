//
//  OutOfSightHidingView.m
//  Blinkenposer
//
//  Created by Dominik Wagner on 27.09.08.
//  Copyright 2008 TheCodingMonkeys. All rights reserved.
//

#import "OutOfSightHidingView.h"


@implementation OutOfSightHidingView

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
    }
    return self;
}

- (void)viewWillMoveToSuperview:(NSView *)newSuperview {
	NSView *oldSuperview = [self superview];
	if (oldSuperview) {
		[[NSNotificationCenter defaultCenter] removeObserver:self name:nil object:oldSuperview];
	}
	if (newSuperview) {
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(superviewFrameDidChange:) name:NSViewFrameDidChangeNotification object:newSuperview]; 
	}
	[newSuperview setPostsFrameChangedNotifications:YES];

	[super viewWillMoveToSuperview:newSuperview];
}

- (void)superviewFrameDidChange:(NSNotification *)aNotification
{
	NSRect bounds = self.bounds;
	NSPoint originPoint = [self convertPoint:bounds.origin toView:nil];
	NSPoint maxXYPoint  = [self convertPoint:NSMakePoint(NSMaxX(bounds),NSMaxY(bounds)) toView:nil];
	NSRect windowRect = [[[self window] contentView] bounds];
//	NSLog(@"%s windowBounds:%@ myBounds:%@ convertetOrigin:%@ convertedMax:%@",__FUNCTION__,
//		NSStringFromRect(windowRect),NSStringFromRect(bounds),NSStringFromPoint(originPoint),NSStringFromPoint(maxXYPoint));
	BOOL completelyVisible = NSPointInRect(originPoint,windowRect) && NSPointInRect(maxXYPoint,windowRect);

	[self setHidden:!completelyVisible];
}

@end
