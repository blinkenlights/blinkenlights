#import "BlinkenSender.h"

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



/* for generating the timstamp later on
	struct timeval tv;
	gettimeofday(&tv,NULL);
	int64_t timeStamp = ((int64_t)tv.tv_sec) * 1000 + tv.tv_usec / 1000;
	NSLog(@"%s sec:%d usec:%d timestamp: %llX",__FUNCTION__,tv.tv_sec,tv.tv_usec,timeStamp);
	
*/

@implementation BlinkenSender

@synthesize targetAddress = I_targetAddress;

+ (NSData *)frameDataForBlinkenStructure:(NSArray *)inBlinkenStructure {
	NSMutableData *result = [NSMutableData data];
	for (NSArray *row in inBlinkenStructure) {
		for (NSNumber *value in row) {
			unsigned char charValue = [value integerValue] * (0xff / 15);
			[result appendBytes:&charValue length:1];
		}
	}
	return result;
}


- (void)createSendSocket {
	I_sendSocket = CFSocketCreate(kCFAllocatorDefault, PF_INET, SOCK_DGRAM, IPPROTO_UDP, 
							   0, NULL, NULL);
	int yes = YES;
	if (I_sendSocket) {
//		NSLog(@"%s having my socket",__FUNCTION__);
		int result = setsockopt(CFSocketGetNative(I_sendSocket), SOL_SOCKET, 
								SO_REUSEADDR, &yes, sizeof(int));
		if (result == -1) {
			NSLog(@"Could not setsockopt to reuseaddr: %@ / %s", errno, strerror(errno));
		}
		
		result = setsockopt(CFSocketGetNative(I_sendSocket), SOL_SOCKET, 
								SO_REUSEPORT, &yes, sizeof(int));
		if (result == -1) {
			NSLog(@"Could not setsockopt to reuseport: %@ / %s", errno, strerror(errno));
		}
		
		CFRunLoopRef currentRunLoop = [[NSRunLoop currentRunLoop] getCFRunLoop];
		CFRunLoopSourceRef runLoopSource = CFSocketCreateRunLoopSource(kCFAllocatorDefault, I_sendSocket, 0);
		CFRunLoopAddSource(currentRunLoop, runLoopSource, kCFRunLoopCommonModes);
		CFRelease(runLoopSource);
	}
}


- (id)init
{
	if ((self=[super init])) {
		self.targetAddress = @"localhost";
		[self createSendSocket];
	}
	return self;
}

- (void)dealloc {
	CFSocketInvalidate(I_sendSocket);
	[I_targetAddress release];
	[super dealloc];
}

- (void)setTargetAddress:(NSString *)inTargetAddress
{
 //   NSLog(@"%s %@",__FUNCTION__,inTargetAddress);
    if (_targetAddressData) {
        CFRelease(_targetAddressData);
        _targetAddressData = nil;
    }

    if (inTargetAddress)
    {
        NSRange range = [inTargetAddress rangeOfString:@":"];
        if (range.location != NSNotFound)
        {
            NSString *portString = [inTargetAddress substringFromIndex:range.location+1];
            _targetPort = [portString integerValue];
            inTargetAddress = [inTargetAddress substringToIndex:range.location];
        } else {
            _targetPort = MCU_LISTENER_PORT;
        }
        //self.targetAddress = inTargetAddress;
        CFDataRef addressData = NULL;
        struct sockaddr_in socketAddress;
        
        NSHost *proxyHost = [NSHost hostWithName:inTargetAddress];
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
        
//        NSLog(@"%s targetHost:%@ address:%@",__FUNCTION__,proxyHost,addressString);
        
        bzero(&socketAddress, sizeof(struct sockaddr_in));
        socketAddress.sin_len = sizeof(struct sockaddr_in);
        socketAddress.sin_family = PF_INET;
        socketAddress.sin_port = htons(_targetPort);
        socketAddress.sin_addr.s_addr = inet_addr([addressString UTF8String]);
        
        addressData = CFDataCreate(kCFAllocatorDefault, (UInt8 *)&socketAddress, sizeof(struct sockaddr_in));
        
        _targetAddressData = addressData;
        NSLog(@"%s did set target address data to: %@",__FUNCTION__,[NSString stringWithAddressData:(NSData *)_targetAddressData]);

    }
    else
    {
        self.targetAddress = nil;
    }

}

- (void)sendData:(NSData *)inData
{
    CFSocketError err = CFSocketSendData (
        I_sendSocket,
        _targetAddressData,
        (CFDataRef)inData,
        0.5
        );
    if (err != kCFSocketSuccess) {
        // NSLog(@"%s could not send data (%d, %@, %u bytes)",__FUNCTION__,err,[NSString stringWithAddressData:(NSData *)_targetAddressData],[inData length]);
        
    } else {
//        NSLog(@"%s did send data: (%@, %u bytes)",__FUNCTION__,[NSString stringWithAddressData:(NSData *)_targetAddressData],[inData length]);
    };
}

- (void)sendBlinkenStructure:(NSArray *)inBlinkenStructure
{
	NSMutableData *sendData = [NSMutableData data];
	
	struct mcu_frame_header header;
	header.magic =    OSSwapHostToBigInt32(MAGIC_MCU_FRAME);
	header.height =   OSSwapHostToBigInt16([inBlinkenStructure count]);
	header.width =    OSSwapHostToBigInt16([[inBlinkenStructure lastObject] count]);
	header.channels = OSSwapHostToBigInt16(1);
	header.maxval =   OSSwapHostToBigInt16(0xff);
	
	[sendData appendBytes:&header length:sizeof(struct mcu_frame_header)];
	
	[sendData appendData:[BlinkenSender frameDataForBlinkenStructure:inBlinkenStructure]];
	
	[self sendData:sendData];
}

@end
