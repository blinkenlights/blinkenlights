//
//  CIRItemSection.h
//  Circulator
//
//  Created by Dominik Wagner on 04.06.08.
//  Copyright 2008 TheCodingMonkeys. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface TableSection : NSObject {
	NSString *I_sectionIndexLabel;
	NSString *I_sectionHeading;
	NSArray  *I_itemArray;
	id I_representedObject;
}

+ (id)sectionWithItems:(NSArray *)aItems heading:(NSString *)aHeading indexLabel:(NSString *)aIndexLabel;

@property (nonatomic,copy)   NSString *indexLabel;
@property (nonatomic,copy)   NSString *heading;
@property (nonatomic,retain) NSArray  *items;
@property (nonatomic,retain) id representedObject;

@end
