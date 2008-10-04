#import "UsefulAdditions.h"

#import <CoreFoundation/CoreFoundation.h>

#import <netinet/in.h>
#import <netinet6/in6.h>
#import <net/if.h>
#import <sys/socket.h>
#import <arpa/inet.h>
#import <unistd.h>
#import <sys/socket.h>

NSString *NSStringFromCGSize(CGSize inSize)
{
	return NSStringFromSize(NSSizeFromCGSize(inSize));
}


@implementation NSString (NSStringNATPortMapperAdditions)
+ (NSString *)stringWithAddressData:(NSData *)aData
{
    if (!aData) return nil;
    struct sockaddr *socketAddress = (struct sockaddr *)[aData bytes];
    
    // IPv6 Addresses are "FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF" at max, which is 40 bytes (0-terminated)
    // IPv4 Addresses are "255.255.255.255" at max which is smaller
    
    char stringBuffer[MAX(INET6_ADDRSTRLEN,INET_ADDRSTRLEN)];
    NSString *addressAsString = nil;
    if (socketAddress->sa_family == AF_INET) {
        if (inet_ntop(AF_INET, &(((struct sockaddr_in *)socketAddress)->sin_addr), stringBuffer, INET_ADDRSTRLEN)) {
            addressAsString = [NSString stringWithUTF8String:stringBuffer];
        } else {
            addressAsString = @"IPv4 un-ntopable";
        }
        int port = ntohs(((struct sockaddr_in *)socketAddress)->sin_port);
            addressAsString = [addressAsString stringByAppendingFormat:@":%d", port];
    } else if (socketAddress->sa_family == AF_INET6) {
         if (inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)socketAddress)->sin6_addr), stringBuffer, INET6_ADDRSTRLEN)) {
            addressAsString = [NSString stringWithUTF8String:stringBuffer];
        } else {
            addressAsString = @"IPv6 un-ntopable";
        }
        int port = ntohs(((struct sockaddr_in6 *)socketAddress)->sin6_port);
        
        // Suggested IPv6 format (see http://www.faqs.org/rfcs/rfc2732.html)
        char interfaceName[IF_NAMESIZE];
        if ([addressAsString hasPrefix:@"fe80"] && if_indextoname(((struct sockaddr_in6 *)socketAddress)->sin6_scope_id,interfaceName)) {
            NSString *zoneID = [NSString stringWithUTF8String:interfaceName];
            addressAsString = [NSString stringWithFormat:@"[%@%%%@]:%d", addressAsString, zoneID, port];
        } else {
            addressAsString = [NSString stringWithFormat:@"[%@]:%d", addressAsString, port];
        }
    } else {
        addressAsString = @"neither IPv6 nor IPv4";
    }
    
    return [[addressAsString copy] autorelease];
}
@end

