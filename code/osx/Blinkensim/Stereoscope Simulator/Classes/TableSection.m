#import "TableSection.h"


@implementation TableSection

@synthesize indexLabel = I_sectionIndexLabel, heading = I_sectionHeading, items = I_itemArray, representedObject = I_representedObject;

+ (id)sectionWithItems:(NSArray *)aItems heading:(NSString *)aHeading indexLabel:(NSString *)aIndexLabel
{
	TableSection *result = (TableSection *)[[TableSection new] autorelease];
	result.items = aItems;
	result.heading = aHeading;
	if (aIndexLabel) {
		result.indexLabel = aIndexLabel;
	} else {
		result.indexLabel = [aHeading substringToIndex:1];
	}
	return result;
}

- (void)dealloc {
	[I_sectionHeading release];
	[I_sectionIndexLabel release];
	[I_itemArray release];
	[super dealloc];
}

@end
