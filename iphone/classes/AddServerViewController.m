/*
 * Copyright 2010 Forwardcode, LLC
 *
 * This file is part of Ventafone.
 *
 * Ventafone is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Ventafone is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Ventafone.  If not, see <http://www.gnu.org/licenses/>.
 */

#import "AddServerViewController.h"
#import "Constants.h"

@implementation AddServerViewController

@synthesize cancelButton;
@synthesize saveButton;
@synthesize myTableView;
@synthesize optionsData;
@synthesize serverIndex;

// // The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
//- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
//	if (!(self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]))
//        return nil;
//	
//    return self;
//}

- (void)dealloc {
	[myTableView release];
	[optionsData release];
	[cancelButton release];
	[saveButton release];
    [super dealloc];
}

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
	appDelegate = (VentafoneAppDelegate *)[[UIApplication sharedApplication] delegate];
	//self.title = NSLocalizedString(@"ModalTitle", @"");
	self.title = @"Add a Server";
	[cancelButton setTitle:@"Cancel"];
	[saveButton setTitle:@"Save"];
	//self.editing = NO;
	
	self.optionsData = [NSMutableArray array];
	UITextField *cellTextField;
	
	/**
	 *	User alias
	 */
	cellTextField = [[UITextField alloc] initWithFrame:CGRectMake(115, 10, 185, 30)];
	cellTextField.adjustsFontSizeToFitWidth = YES;
	cellTextField.textColor = [UIColor blackColor];
	cellTextField.placeholder = @"name yourself";
	cellTextField.clearButtonMode = UITextFieldViewModeWhileEditing;
	cellTextField.keyboardType = UIKeyboardTypeDefault;
	cellTextField.returnKeyType = UIReturnKeyDone;
	cellTextField.tag = 1;
	cellTextField.backgroundColor = [UIColor whiteColor];
	cellTextField.autocorrectionType = UITextAutocorrectionTypeNo; // no auto correction support
	cellTextField.autocapitalizationType = UITextAutocapitalizationTypeNone; // no auto capitalization support
	cellTextField.textAlignment = UITextAlignmentLeft;
	cellTextField.delegate = self;
	[cellTextField setEnabled: YES];
	[optionsData addObject:cellTextField];
	[cellTextField release];
	
	/**
	 *	Host alias
	 */
	cellTextField = [[UITextField alloc] initWithFrame:CGRectMake(115, 10, 185, 30)];
	cellTextField.adjustsFontSizeToFitWidth = YES;
	cellTextField.textColor = [UIColor blackColor];
	cellTextField.placeholder = @"name the server";
	cellTextField.clearButtonMode = UITextFieldViewModeWhileEditing;
	cellTextField.keyboardType = UIKeyboardTypeDefault;
	cellTextField.returnKeyType = UIReturnKeyDone;
	cellTextField.tag = 2;
	cellTextField.backgroundColor = [UIColor whiteColor];
	cellTextField.autocorrectionType = UITextAutocorrectionTypeNo; // no auto correction support
	cellTextField.autocapitalizationType = UITextAutocapitalizationTypeNone; // no auto capitalization support
	cellTextField.textAlignment = UITextAlignmentLeft;
	cellTextField.delegate = self;
	[cellTextField setEnabled: YES];
	[optionsData addObject:cellTextField];
	[cellTextField release];
	
	/**
	 *	Host address
	 */
	cellTextField = [[UITextField alloc] initWithFrame:CGRectMake(115, 10, 185, 30)];
	cellTextField.adjustsFontSizeToFitWidth = YES;
	cellTextField.textColor = [UIColor blackColor];
	cellTextField.placeholder = @"www.example.com";
	cellTextField.clearButtonMode = UITextFieldViewModeWhileEditing;
	cellTextField.keyboardType = UIKeyboardTypeURL;
	cellTextField.returnKeyType = UIReturnKeyDone;
	cellTextField.tag = 3;
	cellTextField.backgroundColor = [UIColor whiteColor];
	cellTextField.autocorrectionType = UITextAutocorrectionTypeNo; // no auto correction support
	cellTextField.autocapitalizationType = UITextAutocapitalizationTypeNone; // no auto capitalization support
	cellTextField.textAlignment = UITextAlignmentLeft;
	cellTextField.delegate = self;
	[cellTextField setEnabled: YES];
	[optionsData addObject:cellTextField];
	[cellTextField release];
	
	/**
	 *	Host port
	 */
	cellTextField = [[UITextField alloc] initWithFrame:CGRectMake(115, 10, 185, 30)];
	cellTextField.adjustsFontSizeToFitWidth = YES;
	cellTextField.textColor = [UIColor blackColor];
	cellTextField.placeholder = @"port address";
	cellTextField.text = @"3784";
	cellTextField.clearButtonMode = UITextFieldViewModeWhileEditing;
	cellTextField.keyboardType = UIKeyboardTypeNumbersAndPunctuation;
	cellTextField.returnKeyType = UIReturnKeyDone;
	cellTextField.tag = 4;
	cellTextField.backgroundColor = [UIColor whiteColor];
	cellTextField.autocorrectionType = UITextAutocorrectionTypeNo; // no auto correction support
	cellTextField.autocapitalizationType = UITextAutocapitalizationTypeNone; // no auto capitalization support
	cellTextField.textAlignment = UITextAlignmentLeft;
	cellTextField.delegate = self;
	[cellTextField setEnabled: YES];
	[optionsData addObject:cellTextField];
	[cellTextField release];
	
	/**
	 *	Host password
	 */
	cellTextField = [[UITextField alloc] initWithFrame:CGRectMake(115, 10, 185, 30)];
	cellTextField.adjustsFontSizeToFitWidth = YES;
	cellTextField.textColor = [UIColor blackColor];
	cellTextField.placeholder = @"Optional";
	cellTextField.clearButtonMode = UITextFieldViewModeWhileEditing;
	cellTextField.keyboardType = UIKeyboardTypeDefault;
	cellTextField.returnKeyType = UIReturnKeyDone;
	cellTextField.secureTextEntry = YES;
	cellTextField.tag = 5;
	cellTextField.backgroundColor = [UIColor whiteColor];
	cellTextField.autocorrectionType = UITextAutocorrectionTypeNo; // no auto correction support
	cellTextField.autocapitalizationType = UITextAutocapitalizationTypeNone; // no auto capitalization support
	cellTextField.textAlignment = UITextAlignmentLeft;
	cellTextField.delegate = self;
	[cellTextField setEnabled: YES];
	[optionsData addObject:cellTextField];
	[cellTextField release];
	
	movedUP = NO;
	[self.myTableView reloadData];
}


/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/

- (void)viewWillAppear:(BOOL)animated
{
	[super viewWillAppear:animated];
	movedUP = NO;
	NSIndexPath *top = [NSIndexPath indexPathForRow:0 inSection:0];
	[self.myTableView scrollToRowAtIndexPath:top atScrollPosition:UITableViewScrollPositionTop animated:NO];
	UITextField *textField;
	
	if (serverIndex >= 0) {
		// Set the values of the settings for the server to be edited
		ServerSettings *server = [[appDelegate serverList] objectAtIndex:serverIndex];
		textField = [optionsData objectAtIndex:0];
		[textField setText:[server userAlias]];
		textField = [optionsData objectAtIndex:1];
		[textField setText:[server hostAlias]];
		textField = [optionsData objectAtIndex:2];
		[textField setText:[server hostAddress]];
		textField = [optionsData objectAtIndex:3];
		[textField setText:[server hostPort]];
		textField = [optionsData objectAtIndex:4];
		[textField setText:[server hostPassword]];
	}
	else {
		// New server, clear all and set defaults
		textField = [optionsData objectAtIndex:0];
		[textField setText:@""];
		textField = [optionsData objectAtIndex:1];
		[textField setText:@""];
		textField = [optionsData objectAtIndex:2];
		[textField setText:@""];
		textField = [optionsData objectAtIndex:3];
		[textField setText:@"3784"];	// Default
		textField = [optionsData objectAtIndex:4];
		[textField setText:@""];
	}
}

- (void)viewDidAppear:(BOOL)animated
{
	[super viewDidAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
	[super viewWillDisappear:animated];
	if (movedUP) {
		self.myTableView.contentInset = UIEdgeInsetsMake(0.0, 0.0, 0.0, 0.0);
		self.myTableView.scrollIndicatorInsets = self.myTableView.contentInset;		
	}
}

- (void)viewDidDisappear:(BOOL)animated
{
	[super viewDidDisappear:animated];
}

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
	self.myTableView = nil;
	self.optionsData = nil;
	[super viewDidUnload];
}

- (BOOL)textFieldShouldReturn:(UITextField *)theTextField {
	
	[theTextField resignFirstResponder];
	[UIView beginAnimations:nil context:NULL];
	[UIView setAnimationDuration:0.4];
	self.myTableView.contentInset = UIEdgeInsetsMake(0.0, 0.0, 0.0, 0.0);
	self.myTableView.scrollIndicatorInsets = self.myTableView.contentInset;
	[UIView commitAnimations];
	NSIndexPath *top = [NSIndexPath indexPathForRow:0 inSection:0];
	[self.myTableView scrollToRowAtIndexPath:top atScrollPosition:UITableViewScrollPositionTop animated:YES];
	movedUP = NO;

	return YES;
}

#pragma mark -
#pragma mark UITableViewDataSource

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
	return 2;
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
	NSString *title = nil;
	switch (section) {
		case 0:
			title = @"Aliases";
			break;
		case 1:
			title = @"Connection Info";
			break;
		default:
			break;
	}
	return title;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
	//Number of rows it should expect should be based on the section
	NSInteger rows = 0;
	switch (section) {
		case 0:
			rows = 2;
			break;
		case 1:
			rows = 3;
			break;
		default:
			break;
	}
	return rows;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
	static NSString *kCellIdentifier = @"kCell";
	
	UITableViewCell *cell = [myTableView dequeueReusableCellWithIdentifier:kCellIdentifier];
	if (cell == nil) {
		cell = [[[UITableViewCell alloc] initWithFrame:CGRectZero reuseIdentifier:kCellIdentifier] autorelease];
		cell.accessoryType = UITableViewCellAccessoryNone;
		[cell setSelectionStyle:UITableViewCellSelectionStyleNone];
		UITextField *cellTextField;
		UISwitch *cellSwitch;
		if ([indexPath section] == 0) { // Email & Password Section
			if ([indexPath row] == 0)
			{
				cell.textLabel.text = @"Your alias";
				cellTextField = [optionsData objectAtIndex:0];
				[cell addSubview:cellTextField];
			}
			else
			{
				cell.textLabel.text = @"Host alias";
				cellTextField = [optionsData objectAtIndex:1];
				[cell addSubview:cellTextField];
			}
		}
		else if ([indexPath section] == 1)
		{
			if ([indexPath row] == 0)
			{
				cell.textLabel.text = @"Address";
				cellTextField = [optionsData objectAtIndex:2];
				[cell addSubview:cellTextField];
			}
			else if ([indexPath row] == 1)
			{
				cell.textLabel.text = @"Port";
				cellTextField = [optionsData objectAtIndex:3];
				[cell addSubview:cellTextField];
			}
			else
			{
				cell.textLabel.text = @"Password";
				cellTextField = [optionsData objectAtIndex:4];
				[cell addSubview:cellTextField];
			}
		}
		
	}
	
	return cell;
}

#pragma mark -
#pragma mark UITableViewDelegate

- (void)textFieldDidBeginEditing:(UITextField *)textField
{
	NSIndexPath *toPath = nil;
	switch ([textField tag]) {
		case 1:
			toPath = [NSIndexPath indexPathForRow:0 inSection:0];
			break;
		case 2:
			toPath = [NSIndexPath indexPathForRow:1 inSection:0];
			break;
		case 3:
			toPath = [NSIndexPath indexPathForRow:0 inSection:1];
			break;
		case 4:
			toPath = [NSIndexPath indexPathForRow:1 inSection:1];
			break;
		case 5:
			toPath = [NSIndexPath indexPathForRow:2 inSection:1];
			break;
		default:
			break;
	}
	
	if (!movedUP) {
		
		[UIView beginAnimations:nil context:NULL];
		[UIView setAnimationDuration:0.4];
		myTableView.contentInset = UIEdgeInsetsMake(0.0, 0.0, 216.0, 0.0);
		myTableView.scrollIndicatorInsets = myTableView.contentInset;
		[UIView commitAnimations];
		movedUP = YES;
	}
//	CGRect rowRect = [myTableView rectForRowAtIndexPath:toPath];
//	CGRect rowRect = [textField frame];
//	NSLog(@"rect x:%f y:%f w:%f h:%f", rowRect.origin.x, rowRect.origin.y, rowRect.size.width, rowRect.size.height);
//	[myTableView scrollRectToVisible:rowRect animated:YES];
	[myTableView scrollToRowAtIndexPath:toPath atScrollPosition:UITableViewScrollPositionTop animated:YES];
	
}

#pragma mark -
#pragma mark API

- (IBAction)cancelAction:(id)sender
{
	[self.parentViewController dismissModalViewControllerAnimated:YES];
}

- (IBAction)saveAction:(id)sender
{
	UITextField *textField;
	UISwitch *switchField;
	
	if (serverIndex >= 0) {
		// Update the settings
		ServerSettings *server = [[appDelegate serverList] objectAtIndex:serverIndex];
		
		textField = [optionsData objectAtIndex:0];
		[server setUserAlias:[textField text]];
		
		textField = [optionsData objectAtIndex:1];
		[server setHostAlias:[textField text]];
		
		textField = [optionsData objectAtIndex:2];
		[server setHostAddress:[textField text]];
		
		textField = [optionsData objectAtIndex:3];
		[server setHostPort:[textField text]];
		
		textField = [optionsData objectAtIndex:4];
		[server setHostPassword:[textField text]];
		
		[(MainViewController *)[[appDelegate navigationController] topViewController] reloadTable];
		
	}
	else {
		// Create a new server settings
		ServerSettings *newServer = [[ServerSettings alloc] init];
		
		textField = [optionsData objectAtIndex:0];
		[newServer setUserAlias:[textField text]];
		
		textField = [optionsData objectAtIndex:1];
		[newServer setHostAlias:[textField text]];
		
		textField = [optionsData objectAtIndex:2];
		[newServer setHostAddress:[textField text]];
		
		textField = [optionsData objectAtIndex:3];
		[newServer setHostPort:[textField text]];
		
		textField = [optionsData objectAtIndex:4];
		[newServer setHostPassword:[textField text]];
		
		[appDelegate addServer:newServer];
		[newServer release];
	}
	
	[appDelegate saveServersToDisk];

	[self.parentViewController dismissModalViewControllerAnimated:YES];
}


@end
