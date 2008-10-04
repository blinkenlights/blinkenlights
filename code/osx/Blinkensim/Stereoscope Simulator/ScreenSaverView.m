//
//  ScreenSaverView.m
//  ScreenSaver
//
//  Created by Martin Pittenauer on 04.10.08.
//  Copyright (c) 2008, TheCodingMonkeys. All rights reserved.
//

#import "ScreenSaverView.h"


@implementation StereoscopeScreenSaverView

- (id)initWithFrame:(NSRect)frame isPreview:(BOOL)isPreview
{
    self = [super initWithFrame:frame isPreview:isPreview];
    if (self) {
        [self setAnimationTimeInterval:1/30.0];
        ogreController = [OgreController new];
        ogreView = [[OgreView alloc] initWithFrame:frame];
        [ogreController setOgreView:ogreView];
        [self addSubview:ogreView];
        [ogreController applicationDidFinishLaunching:nil];
    }
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
    [super drawRect:rect];
}

- (void)animateOneFrame
{
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

@end
