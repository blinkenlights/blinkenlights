//
//  UsefulAdditions.h
//  Blinkenposer
//
//  Created by Dominik Wagner on 13.07.08.
//  Copyright 2008 TheCodingMonkeys. All rights reserved.
//

#import <Foundation/Foundation.h>

NSString *NSStringFromCGSize(CGSize inSize);

@interface NSString (NSStringNATPortMapperAdditions)
+ (NSString *)stringWithAddressData:(NSData *)aData;
@end

