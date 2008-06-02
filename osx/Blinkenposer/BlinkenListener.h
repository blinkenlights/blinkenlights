//
//  BlinkenListener.h
//  Blinkenlistener
//
//  Created by Dominik Wagner on 28.05.08.
//  Copyright 2008 TheCodingMonkeys. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface BlinkenListener : NSObject {
	int	_port;
	id _delegate;
	NSDate *_lastDate;
    CFSocketRef I_listeningSocket;
    CFSocketRef I_listeningSocket6;
    CFSocketRef I_proxySocket;
    int _proxyPort;
    NSString *_proxyAddressString;
    NSTimer *_proxyTimer;
    CFDataRef _proxyAddressData;
}

@property (retain) NSDate *lastDate;
@property (retain) NSString *proxyAddressString;
@property (retain) NSTimer *proxyTimer;


- (void)setProxyAddress:(NSString *)inProxyAddress;
- (void)setPort:(int)aPort;
- (void)listen;
- (void)stopListening;
- (id)delegate;
- (void)setDelegate:(id)inDelegate;
@end


@interface NSObject (BlinkenListenerDelegate)
- (void)receivedFrameData:(NSData *)inFrameData ofSize:(CGSize)inSize channels:(int)inChannels maxValue:(unsigned char)inMaxValue;
@end