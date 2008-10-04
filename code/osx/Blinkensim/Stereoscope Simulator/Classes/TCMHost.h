//
//  TCMHost.h
//

#import <Foundation/Foundation.h>
#import <CFNetwork/CFNetwork.h>

@interface TCMHost : NSObject
{
    id I_delegate;
    CFHostRef I_host;
    NSString *I_name;
    NSMutableArray *I_names;
    unsigned short I_port;
    NSData *I_address;
    NSMutableArray *I_addresses;
    NSDictionary *I_userInfo;
}

+ (TCMHost *)hostWithName:(NSString *)name port:(unsigned short)port userInfo:(NSDictionary *)userInfo;
+ (TCMHost *)hostWithAddressData:(NSData *)addr port:(unsigned short)port userInfo:(NSDictionary *)userInfo;

- (id)initWithName:(NSString *)name port:(unsigned short)port userInfo:(NSDictionary *)userInfo;
- (id)initWithAddressData:(NSData *)addr port:(unsigned short)port userInfo:(NSDictionary *)userInfo;

- (void)setDelegate:(id)delegate;
- (id)delegate;

- (NSArray *)addresses;
- (NSArray *)names;
- (NSDictionary *)userInfo;

- (void)checkReachability;
- (void)resolve;
- (void)reverseLookup;
- (void)cancel;

@end


@interface NSObject (TCMHostDelegateAdditions)

- (void)host:(TCMHost *)sender didNotResolve:(NSError *)error;
- (void)hostDidResolveAddress:(TCMHost *)sender;
- (void)hostDidResolveName:(TCMHost *)sender;

@end