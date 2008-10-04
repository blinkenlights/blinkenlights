//
//  ScreenSaverView.h
//  ScreenSaver
//
//  Created by Martin Pittenauer on 04.10.08.
//  Copyright (c) 2008, TheCodingMonkeys. All rights reserved.
//

#import <ScreenSaver/ScreenSaver.h>
#import <Ogre/OgreOSXCocoaView.h>
#import <Ogre/Ogre.h>
#import "OgreController.h"


@interface StereoscopeScreenSaverView : ScreenSaverView 
{
    OgreController *ogreController;
    OgreView *ogreView;
}

@end
