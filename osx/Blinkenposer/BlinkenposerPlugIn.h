//
//  BlinkenposerPlugIn.h
//  Blinkenposer
//
//  Created by Dominik Wagner on 31.05.08.
//  Copyright (c) 2008 TheCodingMonkeys. All rights reserved.
//

#import <Quartz/Quartz.h>

@interface BlinkenposerPlugIn : QCPlugIn
{
}

/*
Declare here the Obj-C 2.0 properties to be used as input and output ports for the plug-in e.g.
@property double inputFoo;
@property(assign) NSString* outputBar;
You can access their values in the appropriate plug-in methods using self.inputFoo or self.inputBar
*/

@property (assign)id <QCPlugInOutputImageProvider> outputBlinkenImage;

@end
