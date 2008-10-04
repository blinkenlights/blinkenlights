//
//  BlinkenListener.h
//  Blinkenlistener
//
//  Created by Dominik Wagner on 28.05.08.
//  Copyright 2008 TheCodingMonkeys. All rights reserved.
//

#import <Foundation/Foundation.h>

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

@property (nonatomic,retain) NSDate *lastDate;
@property (nonatomic,retain) NSString *proxyAddressString;
@property (nonatomic,retain) NSTimer *proxyTimer;

// if proxy address is set to nil, the listener listens locally on anyip instead of using a proxy. the address can contain a port
- (void)setProxyAddress:(NSString *)inProxyAddress;
- (void)setPort:(int)aPort;
- (void)listen;
- (void)stopListening;
- (id)delegate;
- (void)setDelegate:(id)inDelegate;
@end


@interface NSObject (BlinkenListenerDelegate)
- (void)receivedFrameData:(NSData *)inFrameData ofSize:(CGSize)inSize channels:(int)inChannels maxValue:(unsigned char)inMaxValue;
// timestamp can be 0 to indicate that no timestamp information was present
- (void)blinkenListener:(BlinkenListener *)inListener receivedFrames:(NSArray *)inFrames atTimestamp:(uint64_t)inTimestamp;
@end

@interface BlinkenFrame : NSObject {
	unsigned char _screenID;
	NSData *_frameData;
	CGSize  _frameSize;
	unsigned char _maxValue;
	unsigned char _bitsPerPixel;
}

@property unsigned char maxValue;
@property unsigned char screenID;
@property unsigned char bitsPerPixel;
@property (copy) NSData *frameData;
@property CGSize frameSize;

- (id)initWithData:(NSData *)inData frameSize:(CGSize)inSize screenID:(unsigned char)inScreenID;

@end