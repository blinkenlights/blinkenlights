#import "EditingViewController.h"
#import "AppController.h"

@implementation EditingViewController

@synthesize textValue, editedObject, editedFieldKey, dateEditing, dateValue, textField, dateFormatter, titleString;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if ((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil])) {
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

	[editedObject setValue:textField.text forKey:editedFieldKey];

    [self.navigationController popViewControllerAnimated:YES];
    AppController *appController = [AppController sharedAppController];
    [appController connectToManualProxy];
    [appController doneWithSettings:self];
}

- (BOOL)textFieldShouldReturn:(UITextField *)inTextField {
	[self save:inTextField];
	return YES;
}

- (void)viewWillAppear:(BOOL)animated {
    NSString *capitalizedKey = [self.titleString capitalizedString];
    self.title = capitalizedKey;
    textField.placeholder = capitalizedKey;

	datePicker.hidden = YES;
	textField.enabled = YES;
	textField.text = textValue;
	textField.autocapitalizationType = UITextAutocapitalizationTypeNone;
	textField.autocorrectionType = UITextAutocorrectionTypeNo;
	textField.keyboardType = UIKeyboardTypeURL;
	textField.returnKeyType = UIReturnKeyDone;
	textField.delegate = self;
	[textField becomeFirstResponder];

}

- (IBAction)dateChanged:(id)sender {
    if (dateEditing) textField.text = [dateFormatter stringFromDate:datePicker.date];
}

@end

