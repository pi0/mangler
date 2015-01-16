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

#import "UserViewController.h"


@implementation UserViewController

@synthesize uid;
@synthesize navItem;
@synthesize myTableView;
@synthesize optionsData;

- (void)dealloc {
	[myTableView release];
	[optionsData release];
    [super dealloc];
}

/*
 // The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if ((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil])) {
        // Custom initialization
    }
    return self;
}
*/


// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
	
	appDelegate = (VentafoneAppDelegate *)[[UIApplication sharedApplication] delegate];
	self.optionsData = [NSMutableArray array];
	UISlider *slider;
	UISwitch *cellSwitch;
	
	/**
	 *	Volume slider
	 */
	slider = [[UISlider alloc] initWithFrame:CGRectMake(135, 8, 160, 30)];
	[slider setEnabled:YES];
	[slider setMaximumValue:148.0f];
	[slider setMinimumValue:0.0f];
	[slider setTag:1];
	[optionsData addObject:slider];
	[slider release];
	
	/**
	 *  Mute switch
	 */
	cellSwitch = [[UISwitch alloc] initWithFrame:CGRectMake(205, 8, 80, 30)];
	[cellSwitch setEnabled:YES];
	[cellSwitch setTag:2];
	[cellSwitch addTarget:self action:@selector(switched) forControlEvents:UIControlEventValueChanged];
	[optionsData addObject:cellSwitch];
	[cellSwitch release];
}


- (void)viewWillAppear:(BOOL)animated
{
	[super viewWillAppear:animated];
	[self.navItem setTitle:@"User Settings"];
	
	User *theUser;
	theUser = [appDelegate userForUserId:uid];
	
	NSString *userAlias;
	if (theUser != nil) {
		userAlias = [theUser userAlias];
	}

	NSIndexPath *top = [NSIndexPath indexPathForRow:0 inSection:0];
	[self.myTableView scrollToRowAtIndexPath:top atScrollPosition:UITableViewScrollPositionTop animated:NO];
	UISlider *sliderField;
	UISwitch *switchField;
	
	// Set the interface to that of the settings
	//GlobalSettings *settings = [appDelegate globalSettings];
	sliderField = [optionsData objectAtIndex:0];
	switchField = [optionsData objectAtIndex:1];
	VolumeRecord *theRecord = (VolumeRecord *)[[appDelegate volumeRecords] objectForKey:[theUser userAlias]];
	if (theRecord != nil) {
		// Found a record for this user.. setting slider
		if ([theRecord muted]) {
			// And they are muted
			sliderField.value = 0.0f;
			[sliderField setEnabled:NO];
			[switchField setOn:YES];
		}
		else {
			sliderField.value = (float)[theRecord volume];
			[sliderField setEnabled:YES];
			[switchField setOn:NO];
		}

		
	}
	else
	{
		if (theUser != nil) {
			// Volume record not found.. setting slider to current
			sliderField.value = (float)v3_get_volume_user((uint16_t)[theUser userId]);
			[switchField setOn:NO];
		}
	}
	
	//sliderField.on = [settings keyClicks];
//	switchField = [optionsData objectAtIndex:1];
//	switchField.on = [settings soundEffects];
//	switchField = [optionsData objectAtIndex:2];
//	switchField.on = [settings toggle];
//	switchField = [optionsData objectAtIndex:3];
//	switchField.on = [settings recordOverride];
	
	[self.myTableView reloadData];
}

/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/

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
	return 1;
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
	NSString *title = nil;
	User *theUser = [appDelegate userForUserId:uid];
	switch (section) {
		case 0:
			title = [theUser userAlias];
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
		UISlider *cellSlider;
		UISwitch *cellSwitch;
		if ([indexPath section] == 0) { // Sounds Section
			if ([indexPath row] == 0)
			{
				cell.textLabel.text = @"Volume";
				cellSlider = [optionsData objectAtIndex:0];
				[cell addSubview:cellSlider];
			}
			else if([indexPath row] == 1)
			{
				cell.textLabel.text = @"Mute";
				cellSwitch = [optionsData objectAtIndex:1];
				[cell addSubview:cellSwitch];
			}
		}
		
	}
	
	return cell;
}

#pragma mark -
#pragma mark API

- (IBAction)doneAction:(id)sender
{
	// Save settings to disk
	
	UISlider *sliderField;
	UISwitch *switchField;
	sliderField = [optionsData objectAtIndex:0];
	switchField = [optionsData objectAtIndex:1];
	
	User *theUser = [appDelegate userForUserId:uid];
	
	uint16_t theId = (uint16_t)[theUser userId];
	int theLevel = (int)sliderField.value;
	v3_set_volume_user(theId, theLevel);
	
	VolumeRecord *theRecord = (VolumeRecord *)[[appDelegate volumeRecords] objectForKey:[theUser userAlias]];
	if (theRecord != nil) {
		// Found a record for this user.. so overwrite
		NSInteger volume = (NSInteger)sliderField.value;
		[theRecord setVolume:volume];
		[theRecord setMuted:switchField.on];
		[[appDelegate volumeRecords] setObject:theRecord forKey:[theUser userAlias]];
		
		// Update the user list as well for table display
		[theUser setMuted:switchField.on];
	}
	else {
		// No record found.. create one
		if ([theUser userAlias] != nil) {
			VolumeRecord *aRecord = [[VolumeRecord alloc] init];
			[aRecord setUserAlias:[theUser userAlias]];
			NSInteger volume = (NSInteger)sliderField.value;
			[aRecord setVolume:volume];
			[aRecord setMuted:switchField.on];
			[[appDelegate volumeRecords] setObject:aRecord forKey:[theUser userAlias]];
			
			// Update the user list as well for table display
			[theUser setMuted:switchField.on];
		}		
	}
	
	// Save records to disk
	[appDelegate saveVolumeRecordsToDisk];
	
//	GlobalSettings *settings = [appDelegate globalSettings];
//	
//	switchField = [optionsData objectAtIndex:0];
//	[settings setKeyClicks:switchField.on];
//	
//	switchField = [optionsData objectAtIndex:1];
//	[settings setSoundEffects:switchField.on];
//	
//	switchField = [optionsData objectAtIndex:2];
//	[settings setToggle:switchField.on];
//	
//	switchField = [optionsData objectAtIndex:3];
//	[settings setRecordOverride:switchField.on];
//	
//	[appDelegate saveGlobalSettingsToDisk];
	
	[self.parentViewController dismissModalViewControllerAnimated:YES];
}

- (void)switched
{
	UISlider *cellSlider = [optionsData objectAtIndex:0];
	UISwitch *cellSwitch = [optionsData objectAtIndex:1];
	if (cellSwitch.on) {
		// User turned on mute
		[cellSlider setValue:0.0f];
		[cellSlider setEnabled:NO];
	}
	else {
		// User turned off mute
		[cellSlider setValue:74.0f];
		[cellSlider setEnabled:YES];
		
	}

}


@end
