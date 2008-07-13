//
//  BlinkenListener.m
//  Blinkenlistener
//
//  Created by Dominik Wagner on 28.05.08.
//  Copyright 2008 TheCodingMonkeys. All rights reserved.
//

#import "BlinkenListener.h"

#import <CoreFoundation/CoreFoundation.h>

#import <netinet/in.h>
#import <netinet6/in6.h>
#import <net/if.h>
#import <sys/socket.h>
#import <arpa/inet.h>
#import <unistd.h>
#import <sys/socket.h>

#import "bprotocol.h"

#import "UsefulAdditions.h"

static void readData (
   CFSocketRef aSocket,
   CFSocketCallBackType aCallbackType,
   CFDataRef anAddress,
   const void *data,
   void *info
);

@implementation BlinkenListener

@synthesize lastDate = _lastDate;
@synthesize proxyAddressString = _proxyAddressString;
@synthesize proxyTimer = _proxyTimer;

- (id)init
{
	if ((self=[super init])) {
		_port = 2323;
	}
	
	return self;
}

- (void)dealloc
{
	[self stopListening];
    if (_proxyAddressData) {
        CFRelease(_proxyAddressData);
        _proxyAddressData = nil;
    }
	[_lastDate release];
	[_proxyAddressString release];
	[super dealloc];
}

- (void)setProxyAddress:(NSString *)inProxyAddress
{
    NSLog(@"%s %@",__FUNCTION__,inProxyAddress);
    if (_proxyAddressData) {
        CFRelease(_proxyAddressData);
        _proxyAddressData = nil;
    }

    if (inProxyAddress)
    {
        NSRange range = [inProxyAddress rangeOfString:@":"];
        if (range.location != NSNotFound)
        {
            NSString *portString = [inProxyAddress substringFromIndex:range.location+1];
            _proxyPort = [portString integerValue];
            inProxyAddress = [inProxyAddress substringToIndex:range.location];
        } else {
            _proxyPort = B_HEARTBEAT_PORT;
        }
        self.proxyAddressString = inProxyAddress;
        CFDataRef addressData = NULL;
        struct sockaddr_in socketAddress;
        
        NSHost *proxyHost = [NSHost hostWithName:inProxyAddress];
        NSString *addressString = @"";
        for (NSString *address in [proxyHost addresses])
        {
            // use the first IPv4 address
            if ([address rangeOfString:@"."].location != NSNotFound)
            {
                addressString = address;
                break;
            }
        }
        
        NSLog(@"%s proxyHost:%@ address:%@",__FUNCTION__,proxyHost,addressString);
        
        bzero(&socketAddress, sizeof(struct sockaddr_in));
        socketAddress.sin_len = sizeof(struct sockaddr_in);
        socketAddress.sin_family = PF_INET;
        socketAddress.sin_port = htons(_proxyPort);
        socketAddress.sin_addr.s_addr = inet_addr([addressString UTF8String]);
        
        addressData = CFDataCreate(kCFAllocatorDefault, (UInt8 *)&socketAddress, sizeof(struct sockaddr_in));
        
        _proxyAddressData = addressData;
        NSLog(@"%s did set proxy address data to: %@",__FUNCTION__,[NSString stringWithAddressData:(NSData *)_proxyAddressData]);

    }
    else
    {
        self.proxyAddressString = nil;
    }

    if (I_listeningSocket || I_proxySocket)
    {
        [self stopListening];
        [self listen];
    }
}

- (void)pingTheProxy:(NSTimer *)inTimer
{
    static NSData *s_heartBeatData = nil;
    if (!s_heartBeatData) {
        heartbeat_header_t header;
        header.magic = MAGIC_HEARTBEAT;
        header.version = 0;
        header.padding[0] = 0;
        header.padding[1] = 0;
        header.padding[2] = 0;
        s_heartBeatData = [[NSData alloc] initWithBytes:(char *)&header length:sizeof(heartbeat_header_t)];
    }

    CFSocketError err = CFSocketSendData (
        I_proxySocket,
        _proxyAddressData,
        (CFDataRef)s_heartBeatData,
        0.5
        );
    if (err != kCFSocketSuccess) {
        NSLog(@"%s could not send ping to proxy (%d, %@, %@)",__FUNCTION__,err,[NSString stringWithAddressData:(NSData *)_proxyAddressData],s_heartBeatData);
        
    } else {
        NSLog(@"%s did ping the proxy with: (%@, %@)",__FUNCTION__,[NSString stringWithAddressData:(NSData *)_proxyAddressData],s_heartBeatData);
    };
}



- (void)listen
{
	CFSocketContext socketContext;
	bzero(&socketContext, sizeof(CFSocketContext));
	socketContext.info = self;
	int yes = 1;
	
    NSString *proxyAddressString = self.proxyAddressString;
	
	if (!proxyAddressString)
	{
        I_listeningSocket = CFSocketCreate(kCFAllocatorDefault, PF_INET, SOCK_DGRAM, IPPROTO_UDP, 
                                           kCFSocketDataCallBack, readData, &socketContext);
        if (I_listeningSocket) {
            NSLog(@"%s having my socket",__FUNCTION__);
            int result = setsockopt(CFSocketGetNative(I_listeningSocket), SOL_SOCKET, 
                                    SO_REUSEADDR, &yes, sizeof(int));
            if (result == -1) {
                NSLog(@"Could not setsockopt to reuseaddr: %@ / %s", errno, strerror(errno));
            }
            
            result = setsockopt(CFSocketGetNative(I_listeningSocket), SOL_SOCKET, 
                                    SO_REUSEPORT, &yes, sizeof(int));
            if (result == -1) {
                NSLog(@"Could not setsockopt to reuseport: %@ / %s", errno, strerror(errno));
            }
            
            CFDataRef addressData = NULL;
            struct sockaddr_in socketAddress;
            
            bzero(&socketAddress, sizeof(struct sockaddr_in));
            socketAddress.sin_len = sizeof(struct sockaddr_in);
            socketAddress.sin_family = PF_INET;
            socketAddress.sin_port = htons(_port);
    //		socketAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
            socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
            
            addressData = CFDataCreate(kCFAllocatorDefault, (UInt8 *)&socketAddress, sizeof(struct sockaddr_in));
            if (addressData == NULL) {
                NSLog(@"Could not create addressData");
            } else {
                    
                CFSocketError err = CFSocketSetAddress(I_listeningSocket, addressData);
                if (err != kCFSocketSuccess) {
                    NSLog(@"%s could not set address on socket",__FUNCTION__);
                } else {
                    CFRunLoopRef currentRunLoop = [[NSRunLoop currentRunLoop] getCFRunLoop];
                    CFRunLoopSourceRef runLoopSource = CFSocketCreateRunLoopSource(kCFAllocatorDefault, I_listeningSocket, 0);
                    CFRunLoopAddSource(currentRunLoop, runLoopSource, kCFRunLoopCommonModes);
                    CFRelease(runLoopSource);
                }
                
                CFRelease(addressData);
            }
            addressData = NULL;
    
        }
        
        I_listeningSocket6 = CFSocketCreate(kCFAllocatorDefault, PF_INET6, SOCK_DGRAM, IPPROTO_UDP, 
                                               kCFSocketDataCallBack, readData, &socketContext);
        if (I_listeningSocket6) {
            int result = setsockopt(CFSocketGetNative(I_listeningSocket6), SOL_SOCKET, 
                                    SO_REUSEADDR, &yes, sizeof(int));
            if (result == -1) {
                NSLog(@"%s Could not setsockopt to reuseaddr: %@ / %s", __FUNCTION__, errno, strerror(errno));
            }
            
            result = setsockopt(CFSocketGetNative(I_listeningSocket6), SOL_SOCKET, 
                                    SO_REUSEPORT, &yes, sizeof(int));
            if (result == -1) {
                NSLog(@"%s Could not setsockopt to reuseaddr: %@ / %s", __FUNCTION__, errno, strerror(errno));
            }
            
            struct sockaddr_in6 socketAddress6;
            
            bzero(&socketAddress6, sizeof(struct sockaddr_in6));
            socketAddress6.sin6_len = sizeof(struct sockaddr_in6);
            socketAddress6.sin6_family = PF_INET6;
            socketAddress6.sin6_port = htons(_port);
            memcpy(&(socketAddress6.sin6_addr), &in6addr_any, sizeof(socketAddress6.sin6_addr));
            
            CFDataRef addressData6= CFDataCreate(kCFAllocatorDefault, (UInt8 *)&socketAddress6, sizeof(struct sockaddr_in6));
            if (addressData6 == NULL)
                return;
            
            CFSocketError err = CFSocketSetAddress(I_listeningSocket6, addressData6);
            if (err != kCFSocketSuccess) {
                return;
            }
            
            CFRelease(addressData6);
            addressData6 = NULL;
    
            CFRunLoopRef currentRunLoop = [[NSRunLoop currentRunLoop] getCFRunLoop];
            CFRunLoopSourceRef runLoopSource6 = CFSocketCreateRunLoopSource(kCFAllocatorDefault, I_listeningSocket6, 0);
            CFRunLoopAddSource(currentRunLoop, runLoopSource6, kCFRunLoopCommonModes);
            CFRelease(runLoopSource6);
    
        }
    } else {
        I_proxySocket = CFSocketCreate(kCFAllocatorDefault, PF_INET, SOCK_DGRAM, IPPROTO_UDP, 
                                       kCFSocketDataCallBack, readData, &socketContext);
        if (I_proxySocket) {
            NSLog(@"%s having my socket",__FUNCTION__);
            int result = setsockopt(CFSocketGetNative(I_proxySocket), SOL_SOCKET, 
                                    SO_REUSEADDR, &yes, sizeof(int));
            if (result == -1) {
                NSLog(@"Could not setsockopt to reuseaddr: %@ / %s", errno, strerror(errno));
            }
            
            result = setsockopt(CFSocketGetNative(I_proxySocket), SOL_SOCKET, 
                                    SO_REUSEPORT, &yes, sizeof(int));
            if (result == -1) {
                NSLog(@"Could not setsockopt to reuseport: %@ / %s", errno, strerror(errno));
            }
            
//            CFDataRef addressData = NULL;
//            struct sockaddr_in socketAddress;
//            
//            bzero(&socketAddress, sizeof(struct sockaddr_in));
//            socketAddress.sin_len = sizeof(struct sockaddr_in);
//            socketAddress.sin_family = PF_INET;
//            socketAddress.sin_port = htons(_port);
//            socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
//            
//            addressData = CFDataCreate(kCFAllocatorDefault, (UInt8 *)&socketAddress, sizeof(struct sockaddr_in));
//            if (addressData == NULL) {
//                NSLog(@"Could not create addressData");
//            } else {
                    
            CFRunLoopRef currentRunLoop = [[NSRunLoop currentRunLoop] getCFRunLoop];
            CFRunLoopSourceRef runLoopSource = CFSocketCreateRunLoopSource(kCFAllocatorDefault, I_proxySocket, 0);
            CFRunLoopAddSource(currentRunLoop, runLoopSource, kCFRunLoopCommonModes);
            CFRelease(runLoopSource);
            
            [self pingTheProxy:nil];
            
            self.proxyTimer = [NSTimer scheduledTimerWithTimeInterval:B_HEARTBEAT_INTERVAL / 1000. target:self selector:@selector(pingTheProxy:) userInfo:nil repeats:YES];
    
        }
    
    }	
}

- (void)stopListening
{
//	NSLog(@"%s",__FUNCTION__);
	if (I_listeningSocket) {
	    CFSocketInvalidate(I_listeningSocket);
		I_listeningSocket = nil;
	}
	
	if (I_listeningSocket6) {
		CFSocketInvalidate(I_listeningSocket6);
		I_listeningSocket6 = nil;
	}
	
	[self.proxyTimer invalidate];
	self.proxyTimer = nil;
	
	if (I_proxySocket)
	{
		CFSocketInvalidate(I_proxySocket);
		I_proxySocket = nil;
	}
}

- (void)setPort:(int)aPort
{
	_port = aPort;
}


- (id)delegate
{
	return _delegate;
}

- (void)setDelegate:(id)inDelegate
{
	_delegate = inDelegate;
}

- (void)readData:(NSData *)inData from:(NSString *)inFromAddress
{
//	NSDate *date = [NSDate date];
//	if (self.lastDate) {
//		NSTimeInterval frameTime = [date timeIntervalSinceDate:self.lastDate];
//		NSLog(@"%s since last frame: %f - frame rate of: %f",__FUNCTION__,frameTime, 1./frameTime);
//	} 
//	self.lastDate = date;
//	NSLog(@"%s got %d bytes from sender:%@ ",__FUNCTION__,[inData length],inFromAddress);

	unsigned char *bytes = (unsigned char *)[inData bytes];
	unsigned char *bytesEnd = bytes + [inData length];
	
	uint32_t magic_identifier = ntohl(*((uint32_t *)bytes));
	
	switch (magic_identifier)
	{
		case MAGIC_MCU_SETUP:
			NSLog(@"%s was MAGIC_MCU_SETUP (0x%8X)",__FUNCTION__,magic_identifier);
			break;
		case MAGIC_MCU_FRAME:
		{
			mcu_frame_header_t *header = (void *)bytes;
//			NSLog(@"%s was MAGIC_MCU_FRAME (0x%8X)",__FUNCTION__,magic_identifier);
			header->height   = ntohs(header->height);
			header->width    = ntohs(header->width);
			header->channels = ntohs(header->channels);
			header->maxval   = ntohs(header->maxval);
//			NSLog(@"%s %dx%d %d channels and max of %d",__FUNCTION__, header->width, header->height, header->channels, header->maxval);
			bytes += sizeof(mcu_frame_header_t);
//			int x = 0;
//			int y = 0;
//			for (y = 0; y < (header->height) && bytes < bytesEnd;y++)
//			{
//				for (x = 0; x < (header->width) && bytes < bytesEnd; x++)
//				{
//					printf("%2X",(*(bytes++)));
//				}
//				printf("\n");
//			}
			if ([_delegate respondsToSelector:@selector(receivedFrameData:ofSize:channels:maxValue:)])
			{
				[_delegate receivedFrameData:[NSData dataWithBytes:bytes length:bytesEnd-bytes] ofSize:CGSizeMake(header->width,header->height) channels:header->channels maxValue:header->maxval];
			}
//			NSLog(@"%s bytes: %p, bytesEnd: %p",__FUNCTION__,bytes + header->width*header->height * header->channels,bytesEnd);
			break;
		}
		case MAGIC_MCU_DEVCTRL:
			NSLog(@"%s was MAGIC_MCU_DEVCTRL (0x%8X)",__FUNCTION__,magic_identifier);
			break;
		case MAGIC_BLFRAME:
			NSLog(@"%s was MAGIC_BLFRAME (0x%8X)",__FUNCTION__,magic_identifier);
			break;
		case MAGIC_BLFRAME_256:
			NSLog(@"%s was MAGIC_BLFRAME_256 (0x%8X)",__FUNCTION__,magic_identifier);
			break;
		case MAGIC_HEARTBEAT:
			NSLog(@"%s was MAGIC_HEARTBEAT (0x%8X)",__FUNCTION__,magic_identifier);
			break;
		default:
			NSLog(@"%s magic was to magical (%8X)",__FUNCTION__,magic_identifier);
	}
}

@end


static void readData (
   CFSocketRef aSocket,
   CFSocketCallBackType aCallbackType,
   CFDataRef anAddress,
   const void *aData,
   void *anInfo
) {
    BlinkenListener *blinkenListener = (BlinkenListener *)anInfo;
	NSString *senderAddressAndPort = [NSString stringWithAddressData:(NSData *)anAddress];
	[blinkenListener readData:(NSData *)aData from:senderAddressAndPort];
}

