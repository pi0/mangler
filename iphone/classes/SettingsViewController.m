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

#import "SettingsViewController.h"

@implementation SettingsViewController

@synthesize myTableView;
@synthesize optionsData;

/*
 // The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if ((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil])) {
        // Custom initialization
    }
    return self;
}
*/

- (void)dealloc {
	[myTableView release];
	[optionsData release];
    [super dealloc];
}

/*
// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView {
}
*/

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
	appDelegate = (VentafoneAppDelegate *)[[UIApplication sharedApplication] delegate];
	
	self.optionsData = [NSMutableArray array];
	UISwitch *cellSwitch;
	
	/**
	 *	Play key clicks toggle
	 */
	cellSwitch = [[UISwitch alloc] initWithFrame:CGRectMake(205, 8, 80, 30)];
	[cellSwitch setEnabled:YES];
	[cellSwitch setTag:1];
	[optionsData addObject:cellSwitch];
	[cellSwitch release];
	
	/**
	 *	Play sound effects toggle
	 */
	cellSwitch = [[UISwitch alloc] initWithFrame:CGRectMake(205, 8, 80, 30)];
	[cellSwitch setEnabled:YES];
	[cellSwitch setTag:2];
	[optionsData addObject:cellSwitch];
	[cellSwitch release];
	
	/**
	 *	Talk toggle
	 */
	cellSwitch = [[UISwitch alloc] initWithFrame:CGRectMake(205, 8, 80, 30)];
	[cellSwitch setEnabled:YES];
	[cellSwitch setTag:3];
	[optionsData addObject:cellSwitch];
	[cellSwitch release];
	
	/**
	 *	Override recording quality
	 */
	cellSwitch = [[UISwitch alloc] initWithFrame:CGRectMake(205, 8, 80, 30)];
	[cellSwitch setEnabled:YES];
	[cellSwitch setTag:4];
	[optionsData addObject:cellSwitch];
	[cellSwitch release];
	
	/**
	 *	Small row height toggle
	 */
	cellSwitch = [[UISwitch alloc] initWithFrame:CGRectMake(205, 8, 80, 30)];
	[cellSwitch setEnabled:YES];
	[cellSwitch setTag:5];
	[optionsData addObject:cellSwitch];
	[cellSwitch release];
	
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
	NSIndexPath *top = [NSIndexPath indexPathForRow:0 inSection:0];
	[self.myTableView scrollToRowAtIndexPath:top atScrollPosition:UITableViewScrollPositionTop animated:NO];
	UISwitch *switchField;
	
	// Set the interface to that of the settings
	GlobalSettings *settings = [appDelegate globalSettings];
	switchField = [optionsData objectAtIndex:0];
	switchField.on = [settings keyClicks];
	switchField = [optionsData objectAtIndex:1];
	switchField.on = [settings soundEffects];
	switchField = [optionsData objectAtIndex:2];
	switchField.on = [settings toggle];
	switchField = [optionsData objectAtIndex:3];
	switchField.on = [settings recordOverride];
	switchField = [optionsData objectAtIndex:4];
	switchField.on = [settings smallRowHeight];
}

- (void)viewDidAppear:(BOOL)animated
{
	[super viewDidAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
	[super viewWillDisappear:animated];
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
	self.myTableView = nil;
	self.optionsData = nil;
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

#pragma mark -
#pragma mark UITableViewDataSource

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
	return 3;
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
	NSString *title = nil;
	switch (section) {
		case 0:
			title = @"Sounds";
			break;
		case 1:
			title = @"Recording";
			break;
		case 2:
			title = @"Display";
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
			rows = 2;
			break;
		case 2:
			rows = 1;
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
		UISwitch *cellSwitch;
		if ([indexPath section] == 0) { // Sounds Section
			if ([indexPath row] == 0)
			{
				cell.textLabel.text = @"Talk Clicks";
				cellSwitch = [optionsData objectAtIndex:0];
				[cell addSubview:cellSwitch];
			}
			else {
				cell.textLabel.text = @"Sound Effects";
				cellSwitch = [optionsData objectAtIndex:1];
				[cell addSubview:cellSwitch];
			}

			
		}
		else if ([indexPath section] == 1) { // Recording Section
			if ([indexPath row] == 0)
			{
				cell.textLabel.text = @"Talk Toggle";
				cellSwitch = [optionsData objectAtIndex:2];
				[cell addSubview:cellSwitch];
			}
			else
			{
				cell.textLabel.text = @"Quality Recording";
				cellSwitch = [optionsData objectAtIndex:3];
				[cell addSubview:cellSwitch];
			}
		}
		else if ([indexPath section] == 2) { // Display Section
			if ([indexPath row] == 0) {
				cell.textLabel.text = @"Smaller Row Height";
				cellSwitch = [optionsData objectAtIndex:4];
				[cell addSubview:cellSwitch];
			}
		}
		
	}
	
	return cell;
}

#pragma mark -
#pragma mark API

- (IBAction)cancelAction:(id)sender
{
	[self.parentViewController dismissModalViewControllerAnimated:YES];
}

- (IBAction)saveAction:(id)sender
{
	// Save settings to disk
	
	UISwitch *switchField;
	
	GlobalSettings *settings = [appDelegate globalSettings];
	
	switchField = [optionsData objectAtIndex:0];
	[settings setKeyClicks:switchField.on];
	
	switchField = [optionsData objectAtIndex:1];
	[settings setSoundEffects:switchField.on];
	
	switchField = [optionsData objectAtIndex:2];
	[settings setToggle:switchField.on];
	
	switchField = [optionsData objectAtIndex:3];
	[settings setRecordOverride:switchField.on];
	
	switchField = [optionsData objectAtIndex:4];
	[settings setSmallRowHeight:switchField.on];
	
	[appDelegate saveGlobalSettingsToDisk];

	[self.parentViewController dismissModalViewControllerAnimated:YES];
}



@end
