//
//  BlinkenSender.h
//  Blinkenposer
//
//  Created by Dominik Wagner on 13.07.08.
//  Copyright 2008 TheCodingMonkeys. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface BlinkenSender : NSObject {
	NSString *I_targetAddress;
	CFDataRef _targetAddressData;
    int _targetPort;
    CFSocketRef I_sendSocket;

}

@property (copy) NSString *targetAddress;

+ (NSData *)frameDataForBlinkenStructure:(NSArray *)inBlinkenStructure;
+ (NSData *)frameDataForColorBlinkenStructure:(NSArray *)inBlinkenStructure;
- (void)sendBlinkenStructure:(NSArray *)blinkenStructure; // blinkenstructure is an array of line arrays with values from 0 to 15
- (void)sendBlinkenStructure:(NSArray *)blinkenStructure color:(BOOL)isColor;

@end
