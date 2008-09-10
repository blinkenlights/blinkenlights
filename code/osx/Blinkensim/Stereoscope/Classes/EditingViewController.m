/*

===== IMPORTANT =====

This is sample code demonstrating API, technology or techniques in development.
Although this sample code has been reviewed for technical accuracy, it is not
final. Apple is supplying this information to help you plan for the adoption of
the technologies and programming interfaces described herein. This information
is subject to change, and software implemented based on this sample code should
be tested with final operating system software and final documentation. Newer
versions of this sample code may be provided with future seeds of the API or
technology. For information about updates to this and other developer
documentation, view the New & Updated sidebars in subsequent documentation
seeds.

=====================

File: EditingViewController.m
Abstract: 
Manages editing a single field of data in a Book. Dual mode editor, it supports
both plain text field editing
and date value editing with a UIDatePicker.


Version: 1.6

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc.
("Apple") in consideration of your agreement to the following terms, and your
use, installation, modification or redistribution of this Apple software
constitutes acceptance of these terms.  If you do not agree with these terms,
please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject
to these terms, Apple grants you a personal, non-exclusive license, under
Apple's copyrights in this original Apple software (the "Apple Software"), to
use, reproduce, modify and redistribute the Apple Software, with or without
modifications, in source and/or binary forms; provided that if you redistribute
the Apple Software in its entirety and without modifications, you must retain
this notice and the following text and disclaimers in all such redistributions
of the Apple Software.
Neither the name, trademarks, service marks or logos of Apple Inc. may be used
to endorse or promote products derived from the Apple Software without specific
prior written permission from Apple.  Except as expressly stated in this notice,
no other rights or licenses, express or implied, are granted by Apple herein,
including but not limited to any patent rights that may be infringed by your
derivative works or by other works in which the Apple Software may be
incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR
DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF
CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF
APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Copyright (C) 2008 Apple Inc. All Rights Reserved.

*/

#import "EditingViewController.h"


@implementation EditingViewController

@synthesize textValue, editedObject, editedFieldKey, dateEditing, dateValue, textField, dateFormatter, titleString;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if (self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) {
        // Create a date formatter to convert the date to a string format.
        dateFormatter = [[NSDateFormatter alloc] init];
        [dateFormatter setDateStyle:NSDateFormatterMediumStyle];
        [dateFormatter setTimeStyle:NSDateFormatterNoStyle];
    }
    return self;
}

- (void)viewDidLoad {
    // Adjust the text field size and font.
    CGRect frame = textField.frame;
    frame.size.height += 10;
    textField.frame = frame;
    textField.font = [UIFont boldSystemFontOfSize:16];
    // Set the view background to match the grouped tables in the other views.
    self.view.backgroundColor = [UIColor groupTableViewBackgroundColor];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations.
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

- (void)dealloc {
    [dateFormatter release];
    [datePicker release];
    [textValue release];
    [editedObject release];
    [editedFieldKey release];
    [dateValue release];
    [super dealloc];
}

- (IBAction)cancel:(id)sender {
    // cancel edits
    [self.navigationController popViewControllerAnimated:YES];
}

- (IBAction)save:(id)sender {
    // save edits
    if (dateEditing) {
        [editedObject setValue:datePicker.date forKey:editedFieldKey];
    } else {
        [editedObject setValue:textField.text forKey:editedFieldKey];
    }
//    [editedObject save];
	[[NSNotificationCenter defaultCenter] postNotificationName:@"SettingChange" object:self];

    [self.navigationController popViewControllerAnimated:YES];
}

- (void)viewWillAppear:(BOOL)animated {
    NSString *capitalizedKey = [self.titleString capitalizedString];
    self.title = capitalizedKey;
    textField.placeholder = capitalizedKey;
    if (dateEditing) {
        datePicker.hidden = NO;
        textField.enabled = NO;
        textField.text = [dateFormatter stringFromDate:dateValue];
        datePicker.date = dateValue;
    } else {
        datePicker.hidden = YES;
        textField.enabled = YES;
        textField.text = textValue;
        textField.autocapitalizationType = UITextAutocapitalizationTypeNone;
        textField.autocorrectionType = UITextAutocorrectionTypeNo;
        textField.keyboardType = UIKeyboardTypeURL;
        textField.returnKeyType = UIReturnKeyDone;
        [textField becomeFirstResponder];
    }
}

- (IBAction)dateChanged:(id)sender {
    if (dateEditing) textField.text = [dateFormatter stringFromDate:datePicker.date];
}

@end

