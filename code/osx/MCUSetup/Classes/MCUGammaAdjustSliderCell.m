//
//  MCUGammaAdjustSliderCell.m
//  MCUSetup
//
//  Created by Dominik Wagner on 30.09.08.
//  Copyright 2008 TheCodingMonkeys. All rights reserved.
//

#import "MCUGammaAdjustSliderCell.h"

#define VALUE_FONT_SIZE			10
#define SHADE_NUMBER_FONT_SIZE	10

#define LABELWIDTH 40.0
#define XINSET 10.0
#define YINSET 4.0
#define BUTTONWIDTH 40.0
#define BUTTONSTEP 50
@implementation MCUGammaAdjustSliderCell

@synthesize shadeValue;
@synthesize shadeNumber;
@synthesize delegate;

- (id)initWithFrame:(CGRect)frame reuseIdentifier:(NSString *)reuseIdentifier {
    if (self = [super initWithFrame:frame reuseIdentifier:reuseIdentifier]) {
        // Initialization code
		valueLabel = [[UILabel alloc] initWithFrame:CGRectZero];
        [valueLabel setFont:[UIFont systemFontOfSize:VALUE_FONT_SIZE]];
        [self.contentView addSubview:valueLabel];

		numberLabel = [[UILabel alloc] initWithFrame:CGRectZero];
        [numberLabel setFont:[UIFont boldSystemFontOfSize:SHADE_NUMBER_FONT_SIZE]];
        [self.contentView addSubview:numberLabel];

		valueSlider = [[UISlider alloc] initWithFrame:CGRectZero];
		[valueSlider addTarget:self action:@selector(sliderAction:) forControlEvents:UIControlEventValueChanged];
		[valueSlider addTarget:self action:@selector(sliderActionUp:) forControlEvents:UIControlEventTouchUpInside];
		valueSlider.minimumValue = 2300;
		valueSlider.maximumValue = 6700;
		valueSlider.continuous = YES;
		[self.contentView addSubview:valueSlider];
		
		plusButton = [[UIButton buttonWithType:UIButtonTypeRoundedRect] retain];
		plusButton.frame = CGRectZero;
		[plusButton setTitle:@"+" forState:UIControlStateNormal];
		[plusButton addTarget:self action:@selector(plusAction) forControlEvents:UIControlEventTouchUpInside];
		[self.contentView addSubview:plusButton];

		minusButton = [[UIButton buttonWithType:UIButtonTypeRoundedRect] retain];
		minusButton.frame = CGRectZero;
		[minusButton setTitle:@"-" forState:UIControlStateNormal];
		[minusButton addTarget:self action:@selector(minusAction) forControlEvents:UIControlEventTouchUpInside];
		[self.contentView addSubview:minusButton];

    }
    return self;
}

- (void)plusAction {
	self.shadeValue = MIN(shadeValue + BUTTONSTEP,10000);
	if ([delegate respondsToSelector:@selector(sliderCellDidChangeValue:)]) {
		[delegate sliderCellDidChangeValue:self];
	}
}

- (void)minusAction {
	self.shadeValue = MAX(shadeValue - BUTTONSTEP,0);
	if ([delegate respondsToSelector:@selector(sliderCellDidChangeValue:)]) {
		[delegate sliderCellDidChangeValue:self];
	}
}

- (void)setShadeValue:(int)inShadeValue
{
	shadeValue = inShadeValue;
	valueLabel.text = [NSString stringWithFormat:@"%d", shadeValue];
	valueSlider.value = inShadeValue;
}

- (void)setShadeNumber:(int)inShadeNumber
{
	shadeNumber = inShadeNumber;
	numberLabel.text = [NSString stringWithFormat:@"%d", inShadeNumber];
}

- (void)sliderAction:(id)inSender
{
	shadeValue = (int)round([valueSlider value]);
	valueLabel.text = [NSString stringWithFormat:@"%d", shadeValue];
//	NSLog(@"%s %d",__FUNCTION__,shadeValue);
}

- (void)sliderActionUp:(id)inSender
{
	if ([delegate respondsToSelector:@selector(sliderCellDidChangeValue:)]) {
		[delegate sliderCellDidChangeValue:self];
	}
//	NSLog(@"%s %d",__FUNCTION__,(int)round([valueSlider value]));
}


- (CGRect)numberLabelFrame {
	CGRect bounds = self.contentView.bounds;
	bounds.origin.x = bounds.size.width - XINSET;
	bounds.size.height = (bounds.size.height/2 - YINSET);
	bounds.size.width = LABELWIDTH;
	bounds.origin.y = YINSET;
	bounds.origin.x -= bounds.size.width;
	return bounds;
}

- (CGRect)valueLabelFrame {
	CGRect bounds = self.contentView.bounds;
	bounds.origin.x = bounds.size.width - XINSET;
	bounds.origin.y = bounds.size.height/2;
	bounds.size.height = (bounds.size.height/2 - YINSET);
	bounds.size.width = LABELWIDTH;
	bounds.origin.x -= bounds.size.width;
	return bounds;
}

- (CGRect)valueSliderFrame {
	CGRect bounds = self.contentView.bounds;
	bounds.size.height = (bounds.size.height- 2*YINSET);
	bounds.size.width = bounds.size.width - LABELWIDTH - 5*XINSET - BUTTONWIDTH*2;
	bounds.origin.x = XINSET * 2 + BUTTONWIDTH;
	bounds.origin.y = YINSET;
	return bounds;
}

- (CGRect)plusButtonFrame {
	CGRect bounds = self.contentView.bounds;
	bounds.size.height = bounds.size.height - 2*YINSET;
	bounds.origin.x = bounds.size.width - 2 * XINSET - BUTTONWIDTH - LABELWIDTH;
	bounds.size.width = BUTTONWIDTH;
	bounds.origin.y = YINSET;
	return bounds;
}

- (CGRect)minusButtonFrame {
	CGRect bounds = self.contentView.bounds;
	bounds.size.height = bounds.size.height - 2*YINSET;
	bounds.size.width = BUTTONWIDTH;
	bounds.origin.x = XINSET;
	bounds.origin.y = YINSET;
	return bounds;
}

- (void)layoutSubviews {
	[super layoutSubviews];
	[minusButton setFrame:[self minusButtonFrame]];
	[plusButton setFrame:[self plusButtonFrame]];
	[valueLabel setFrame:[self valueLabelFrame]];
	[numberLabel setFrame:[self numberLabelFrame]];
	[valueSlider setFrame:[self valueSliderFrame]];
}


- (void)setSelected:(BOOL)selected animated:(BOOL)animated {

    [super setSelected:selected animated:animated];

    // Configure the view for the selected state
}

- (void)dealloc {
    [super dealloc];
}


@end
