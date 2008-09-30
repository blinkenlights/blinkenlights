//
//  MCUGammaAdjustSliderCell.h
//  MCUSetup
//
//  Created by Dominik Wagner on 30.09.08.
//  Copyright 2008 TheCodingMonkeys. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface MCUGammaAdjustSliderCell : UITableViewCell {
	UISlider *valueSlider;
	UILabel  *valueLabel;
	UILabel  *numberLabel;
	int shadeNumber;
	int shadeValue;
	id delegate;
	UIButton *minusButton;
	UIButton *plusButton;
}

@property int shadeNumber;
@property int shadeValue;
@property (nonatomic,assign) id delegate;

@end

@interface NSObject (MCUGammaAdjustSliderCellDelegate)
- (void)sliderCellDidChangeValue:(MCUGammaAdjustSliderCell *)inCell;
@end
